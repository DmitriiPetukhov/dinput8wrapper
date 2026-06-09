#pragma once

#include <math.h>

struct XInputRumbleFrame
{
	WORD left;
	WORD right;
};

class CDirectInputEffectXInput : public IDirectInputEffect
{
private:
	volatile LONG refCount;
	DWORD userIndex;
	GUID effectGuid;
	DIEFFECT effect;
	BYTE typeSpecificStorage[64];
	bool active;
	bool downloaded;
	ULONGLONG startMs;
	DWORD iterations;

	DWORD ClampGain(DWORD gain)
	{
		return gain > 10000 ? 10000 : gain;
	}

	DWORD AbsMagnitude(LONG magnitude)
	{
		if (magnitude == LONG_MIN)
		{
			return 10000;
		}

		LONG positive = magnitude < 0 ? -magnitude : magnitude;
		return positive > 10000 ? 10000 : (DWORD)positive;
	}

	DWORD ExpectedTypeSpecificSize()
	{
		if (IsEqualIID(effectGuid, GUID_ConstantForce))
		{
			return sizeof(DICONSTANTFORCE);
		}

		if (IsEqualIID(effectGuid, GUID_RampForce))
		{
			return sizeof(DIRAMPFORCE);
		}

		if (IsEqualIID(effectGuid, GUID_Sine) ||
			IsEqualIID(effectGuid, GUID_Square) ||
			IsEqualIID(effectGuid, GUID_Triangle) ||
			IsEqualIID(effectGuid, GUID_SawtoothUp) ||
			IsEqualIID(effectGuid, GUID_SawtoothDown))
		{
			return sizeof(DIPERIODIC);
		}

		return 0;
	}

	double ClampStrength(double strength)
	{
		if (strength < 0.0)
		{
			return 0.0;
		}

		return strength > 1.0 ? 1.0 : strength;
	}

	double ClampSignedMagnitude(double magnitude)
	{
		if (magnitude < -10000.0)
		{
			return -10000.0;
		}

		if (magnitude > 10000.0)
		{
			return 10000.0;
		}

		return magnitude;
	}

	double EvaluatePeriodicStrength(ULONGLONG elapsedUs)
	{
		DIPERIODIC* params = (DIPERIODIC*)effect.lpvTypeSpecificParams;
		double period = params->dwPeriod ? (double)params->dwPeriod : 100000.0;
		double phase = fmod((double)elapsedUs, period) / period;
		phase += params->dwPhase / 36000.0;
		phase = phase - floor(phase);

		double wave = 0.0;
		if (IsEqualIID(effectGuid, GUID_Sine))
		{
			wave = sin(phase * 6.283185307179586);
		}
		else if (IsEqualIID(effectGuid, GUID_Square))
		{
			wave = phase < 0.5 ? 1.0 : -1.0;
		}
		else if (IsEqualIID(effectGuid, GUID_Triangle))
		{
			wave = phase < 0.5 ? (phase * 4.0 - 1.0) : (3.0 - phase * 4.0);
		}
		else if (IsEqualIID(effectGuid, GUID_SawtoothDown))
		{
			wave = 1.0 - phase * 2.0;
		}
		else
		{
			wave = phase * 2.0 - 1.0;
		}

		double magnitude = params->lOffset + (wave * min(params->dwMagnitude, (DWORD)10000));
		return fabs(ClampSignedMagnitude(magnitude)) / 10000.0;
	}

	double EvaluateRampStrength(ULONGLONG elapsedUs)
	{
		DIRAMPFORCE* params = (DIRAMPFORCE*)effect.lpvTypeSpecificParams;
		double durationUs = effect.dwDuration == INFINITE || effect.dwDuration == 0 ? 1000000.0 : (double)effect.dwDuration;
		double position = elapsedUs / durationUs;
		if (position > 1.0)
		{
			position = 1.0;
		}

		double magnitude = params->lStart + ((params->lEnd - params->lStart) * position);
		return fabs(ClampSignedMagnitude(magnitude)) / 10000.0;
	}

	HRESULT CopyEffectParameters(LPCDIEFFECT lpeff)
	{
		if (!lpeff || lpeff->dwSize < sizeof(DIEFFECT))
		{
			return DIERR_INVALIDPARAM;
		}

		DWORD expectedSize = ExpectedTypeSpecificSize();
		if (lpeff->cbTypeSpecificParams > sizeof(typeSpecificStorage) ||
			(expectedSize != 0 && lpeff->cbTypeSpecificParams != expectedSize) ||
			(lpeff->cbTypeSpecificParams != 0 && !lpeff->lpvTypeSpecificParams))
		{
			return DIERR_INVALIDPARAM;
		}

		effect = *lpeff;
		effect.dwGain = ClampGain(effect.dwGain);
		if (effect.dwTriggerButton == 0)
		{
			effect.dwTriggerButton = DIEB_NOTRIGGER;
		}

		if (lpeff->cbTypeSpecificParams != 0)
		{
			memcpy(typeSpecificStorage, lpeff->lpvTypeSpecificParams, lpeff->cbTypeSpecificParams);
			effect.lpvTypeSpecificParams = typeSpecificStorage;
		}

		return DI_OK;
	}

public:
	CDirectInputEffectXInput(DWORD userIndex, GUID* rguid)
	{
		refCount = 1;
		this->userIndex = userIndex;
		effectGuid = rguid ? *rguid : GUID_ConstantForce;
		ZeroMemory(&effect, sizeof(effect));
		ZeroMemory(typeSpecificStorage, sizeof(typeSpecificStorage));
		effect.dwSize = sizeof(DIEFFECT);
		effect.dwDuration = INFINITE;
		effect.dwGain = 10000;
		effect.dwTriggerButton = DIEB_NOTRIGGER;
		active = false;
		downloaded = false;
		startMs = 0;
		iterations = 0;
	}

	HRESULT STDMETHODCALLTYPE QueryInterface(GUID* riid, LPVOID* ppvObj)
	{
		if (!ppvObj)
		{
			return DIERR_INVALIDPARAM;
		}

		*ppvObj = NULL;
		if (!riid)
		{
			return DIERR_INVALIDPARAM;
		}

		if (IsEqualIID(*riid, IID_IUnknown) || IsEqualIID(*riid, IID_IDirectInputEffect))
		{
			*ppvObj = static_cast<IDirectInputEffect*>(this);
			AddRef();
			return DI_OK;
		}

		return DIERR_NOINTERFACE;
	}

	ULONG STDMETHODCALLTYPE AddRef()
	{
		return (ULONG)InterlockedIncrement(&refCount);
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		LONG currentRefCount = InterlockedDecrement(&refCount);
		if (currentRefCount == 0)
		{
			Stop();
			delete this;
		}

		return (ULONG)currentRefCount;
	}

	HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE, DWORD, GUID*)
	{
		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE GetEffectGuid(LPGUID pguid)
	{
		if (!pguid)
		{
			return DIERR_INVALIDPARAM;
		}

		*pguid = effectGuid;
		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE GetParameters(LPDIEFFECT lpeff, DWORD)
	{
		if (!lpeff || lpeff->dwSize < sizeof(DIEFFECT))
		{
			return DIERR_INVALIDPARAM;
		}

		DWORD callerSize = lpeff->dwSize;
		*lpeff = effect;
		lpeff->dwSize = callerSize;
		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE SetParameters(LPCDIEFFECT lpeff, DWORD flags)
	{
		HRESULT hr = CopyEffectParameters(lpeff);
		if (FAILED(hr))
		{
			return hr;
		}

		downloaded = true;
		return (flags & DIEP_START) ? Start(1, 0) : DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Start(DWORD iterations, DWORD)
	{
		if (!diGlobalsInstance->IsXInputControllerConnected(userIndex))
		{
			return DIERR_UNPLUGGED;
		}

		this->iterations = iterations;
		startMs = GetTickCount64();
		downloaded = true;
		active = true;
		HRESULT hr = diGlobalsInstance->RegisterActiveEffect(this);
		if (FAILED(hr))
		{
			active = false;
			return hr;
		}

		diGlobalsInstance->RecomputeAndApplyRumble(userIndex);
		diGlobalsInstance->EnsureHapticsThreadStarted();
		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Stop()
	{
		if (active)
		{
			active = false;
			diGlobalsInstance->UnregisterActiveEffect(this);
			diGlobalsInstance->RecomputeAndApplyRumble(userIndex);
		}

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE GetEffectStatus(LPDWORD pdwFlags)
	{
		if (!pdwFlags)
		{
			return DIERR_INVALIDPARAM;
		}

		*pdwFlags = DIEGES_EMULATED;
		if (active)
		{
			*pdwFlags |= DIEGES_PLAYING;
		}

		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Download()
	{
		downloaded = true;
		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Unload()
	{
		Stop();
		downloaded = false;
		return DI_OK;
	}

	HRESULT STDMETHODCALLTYPE Escape(LPDIEFFESCAPE pesc)
	{
		return pesc ? DIERR_UNSUPPORTED : DIERR_INVALIDPARAM;
	}

	XInputRumbleFrame Evaluate(ULONGLONG nowMs)
	{
		XInputRumbleFrame frame = {};
		if (!active || !downloaded || !effect.lpvTypeSpecificParams)
		{
			return frame;
		}

		double gain = ClampGain(effect.dwGain) / 10000.0;
		double strength = 0.0;
		ULONGLONG elapsedUs = (nowMs - startMs) * 1000;

		if (IsEqualIID(effectGuid, GUID_ConstantForce))
		{
			DICONSTANTFORCE* params = (DICONSTANTFORCE*)effect.lpvTypeSpecificParams;
			strength = AbsMagnitude(params->lMagnitude) / 10000.0;
		}
		else if (IsEqualIID(effectGuid, GUID_RampForce))
		{
			strength = EvaluateRampStrength(elapsedUs);
		}
		else if (IsEqualIID(effectGuid, GUID_Sine) ||
			IsEqualIID(effectGuid, GUID_Square) ||
			IsEqualIID(effectGuid, GUID_Triangle) ||
			IsEqualIID(effectGuid, GUID_SawtoothUp) ||
			IsEqualIID(effectGuid, GUID_SawtoothDown))
		{
			strength = EvaluatePeriodicStrength(elapsedUs);
		}

		strength = ClampStrength(strength);
		WORD left = (WORD)min(65535.0, strength * gain * 65535.0);
		WORD right = (WORD)min(65535.0, strength * gain * 45000.0);
		frame.left = left;
		frame.right = right;
		return frame;
	}

	DWORD GetUserIndex()
	{
		return userIndex;
	}

	bool IsActive()
	{
		return active;
	}

	bool HasExpired(ULONGLONG nowMs)
	{
		if (!active || effect.dwDuration == INFINITE)
		{
			return false;
		}

		DWORD playIterations = iterations == 0 ? 1 : iterations;
		ULONGLONG elapsedUs = (nowMs - startMs) * 1000;
		return elapsedUs >= ((ULONGLONG)effect.dwDuration * playIterations);
	}
};

inline HRESULT CDirectInput8Globals::RegisterActiveEffect(CDirectInputEffectXInput* effect)
{
	if (!effect)
	{
		return DIERR_INVALIDPARAM;
	}

	HRESULT result = DI_OK;
	bool found = false;
	Lock();
	{
		for (DWORD i = 0; i < activeEffectCount; i++)
		{
			if (activeEffects[i] == effect)
			{
				found = true;
				break;
			}
		}

		if (!found && activeEffectCount < ARRAYSIZE(activeEffects))
		{
			activeEffects[activeEffectCount++] = effect;
		}
		else if (!found)
		{
			result = DIERR_INVALIDPARAM;
		}

		if (hapticsWakeEvent)
		{
			SetEvent(hapticsWakeEvent);
		}
	}
	Unlock();

	return result;
}

inline void CDirectInput8Globals::UnregisterActiveEffect(CDirectInputEffectXInput* effect)
{
	if (!effect)
	{
		return;
	}

	Lock();
	{
		for (DWORD i = 0; i < activeEffectCount; i++)
		{
			if (activeEffects[i] == effect)
			{
				for (DWORD j = i; j + 1 < activeEffectCount; j++)
				{
					activeEffects[j] = activeEffects[j + 1];
				}

				activeEffectCount--;
				activeEffects[activeEffectCount] = NULL;
				break;
			}
		}

		if (hapticsWakeEvent)
		{
			SetEvent(hapticsWakeEvent);
		}
	}
	Unlock();

	if (activeEffectCount == 0 && hapticsThreadRunning)
	{
		if (GetCurrentThreadId() == hapticsThreadId)
		{
			hapticsThreadRunning = false;
		}
		else
		{
			StopHapticsThread();
		}
	}
}

inline void CDirectInput8Globals::RecomputeAndApplyRumble(DWORD userIndex)
{
	WORD left = 0;
	WORD right = 0;
	ULONGLONG nowMs = GetTickCount64();

	Lock();
	{
		if (!IsControllerForceFeedbackMuted(userIndex))
		{
			for (DWORD i = 0; i < activeEffectCount; i++)
			{
				if (!activeEffects[i] || activeEffects[i]->GetUserIndex() != userIndex)
				{
					continue;
				}

				XInputRumbleFrame frame = activeEffects[i]->Evaluate(nowMs);
				left = max(left, frame.left);
				right = max(right, frame.right);
			}
		}
	}
	Unlock();

	SetControllerVibration(userIndex, left, right);
}

inline void CDirectInput8Globals::SetControllerForceFeedbackPaused(DWORD userIndex, bool paused)
{
	if (userIndex >= 4)
	{
		return;
	}

	Lock();
	{
		controllerForceFeedbackPaused[userIndex] = paused;
	}
	Unlock();
}

inline void CDirectInput8Globals::SetControllerForceFeedbackActuatorsEnabled(DWORD userIndex, bool enabled)
{
	if (userIndex >= 4)
	{
		return;
	}

	Lock();
	{
		controllerForceFeedbackActuatorsEnabled[userIndex] = enabled;
	}
	Unlock();
}

inline bool CDirectInput8Globals::IsControllerForceFeedbackMuted(DWORD userIndex)
{
	if (userIndex >= 4)
	{
		return true;
	}

	return controllerForceFeedbackPaused[userIndex] || !controllerForceFeedbackActuatorsEnabled[userIndex];
}

inline void CDirectInput8Globals::EnsureHapticsThreadStarted()
{
	if (hapticsThreadRunning)
	{
		if (hapticsWakeEvent)
		{
			SetEvent(hapticsWakeEvent);
		}

		return;
	}

	hapticsThreadRunning = true;
	hapticsThread = CreateThread(NULL, 0, HapticsThreadProc, this, 0, &hapticsThreadId);
	if (!hapticsThread)
	{
		hapticsThreadRunning = false;
		hapticsThreadId = 0;
	}
}

inline void CDirectInput8Globals::StopHapticsThread()
{
	if (!hapticsThreadRunning && !hapticsThread)
	{
		return;
	}

	hapticsThreadRunning = false;
	if (hapticsWakeEvent)
	{
		SetEvent(hapticsWakeEvent);
	}

	if (hapticsThread && GetCurrentThreadId() != hapticsThreadId)
	{
		WaitForSingleObject(hapticsThread, 1000);
		CloseHandle(hapticsThread);
	}

	hapticsThread = NULL;
	hapticsThreadId = 0;
}

inline void CDirectInput8Globals::UpdateAllActiveRumble()
{
	CDirectInputEffectXInput* effects[32];
	DWORD effectCount = 0;
	bool touchedControllers[4] = {};
	ULONGLONG nowMs = GetTickCount64();

	Lock();
	{
		effectCount = activeEffectCount;
		for (DWORD i = 0; i < effectCount; i++)
		{
			effects[i] = activeEffects[i];
			if (effects[i] && effects[i]->GetUserIndex() < 4)
			{
				touchedControllers[effects[i]->GetUserIndex()] = true;
			}
		}
	}
	Unlock();

	for (DWORD i = 0; i < effectCount; i++)
	{
		if (!effects[i])
		{
			continue;
		}

		DWORD userIndex = effects[i]->GetUserIndex();
		if (!IsXInputControllerConnected(userIndex) || effects[i]->HasExpired(nowMs))
		{
			effects[i]->Stop();
			StopControllerVibration(userIndex);
			touchedControllers[userIndex] = true;
		}
	}

	for (DWORD i = 0; i < 4; i++)
	{
		if (touchedControllers[i])
		{
			RecomputeAndApplyRumble(i);
		}
	}
}

inline DWORD WINAPI CDirectInput8Globals::HapticsThreadProc(LPVOID context)
{
	CDirectInput8Globals* globals = (CDirectInput8Globals*)context;
	while (globals->hapticsThreadRunning)
	{
		globals->UpdateAllActiveRumble();
		WaitForSingleObject(globals->hapticsWakeEvent, 20);
	}

	return 0;
}

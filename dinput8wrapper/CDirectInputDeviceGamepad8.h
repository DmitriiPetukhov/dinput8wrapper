#pragma once

class CDirectInputDeviceGamepad8
{
public:
	DIDATAFORMAT dataFormat;
	DIOBJECTDATAFORMAT objectDataFormat[256];
	ULONG refCount;
	DWORD dwDevType;
	DWORD xinputUserIndex;
	bool isAcquired;
	LPDIRECTINPUTEFFECT createdEffects[32];
	DWORD createdEffectCount;
	bool forceFeedbackPaused;
	bool forceFeedbackActuatorsEnabled;

	CDirectInputDeviceGamepad8(DWORD userIndex)
	{
		ZeroMemory(&dataFormat, sizeof(dataFormat));
		ZeroMemory(objectDataFormat, sizeof(objectDataFormat));
		refCount = 1;
		xinputUserIndex = userIndex;
		isAcquired = false;
		ZeroMemory(createdEffects, sizeof(createdEffects));
		createdEffectCount = 0;
		forceFeedbackPaused = false;
		forceFeedbackActuatorsEnabled = true;
	}

	bool ShouldEnumObject(DWORD requestedFlags, DWORD objectType)
	{
		if (requestedFlags == DIDFT_ALL)
		{
			return true;
		}

		DWORD requestedInstance = (requestedFlags >> 8) & 0xFFFF;
		DWORD objectInstance = (objectType >> 8) & 0xFFFF;
		if (requestedInstance != 0 && requestedInstance != objectInstance)
		{
			return false;
		}

		DWORD requestedType = requestedFlags & 0xFF;
		DWORD objectTypeFlags = objectType & 0xFF;

		if (requestedType == objectTypeFlags)
		{
			return true;
		}

		if (requestedType == DIDFT_AXIS && (objectTypeFlags == DIDFT_ABSAXIS || objectTypeFlags == DIDFT_RELAXIS))
		{
			return true;
		}

		if (requestedType == DIDFT_BUTTON && (objectTypeFlags == DIDFT_PSHBUTTON || objectTypeFlags == DIDFT_TGLBUTTON))
		{
			return true;
		}

		return false;
	}

	bool IsSupportedEffectGuid(GUID* rguid)
	{
		return rguid &&
			(IsEqualIID(*rguid, GUID_ConstantForce) ||
				IsEqualIID(*rguid, GUID_RampForce) ||
				IsEqualIID(*rguid, GUID_Sine) ||
				IsEqualIID(*rguid, GUID_Square) ||
				IsEqualIID(*rguid, GUID_Triangle) ||
				IsEqualIID(*rguid, GUID_SawtoothUp) ||
				IsEqualIID(*rguid, GUID_SawtoothDown));
	}

	DWORD GetSourceOffsetForObject(const DIOBJECTDATAFORMAT* objectFormat)
	{
		DWORD objectType = objectFormat->dwType;
		DWORD objectInstance = (objectType >> 8) & 0xFFFF;

		if (objectFormat->pguid)
		{
			if (IsEqualIID(*objectFormat->pguid, GUID_XAxis)) return DIJOFS_X;
			if (IsEqualIID(*objectFormat->pguid, GUID_YAxis)) return DIJOFS_Y;
			if (IsEqualIID(*objectFormat->pguid, GUID_ZAxis)) return DIJOFS_Z;
			if (IsEqualIID(*objectFormat->pguid, GUID_RxAxis)) return DIJOFS_RX;
			if (IsEqualIID(*objectFormat->pguid, GUID_RyAxis)) return DIJOFS_RY;
			if (IsEqualIID(*objectFormat->pguid, GUID_RzAxis)) return DIJOFS_RZ;
			if (IsEqualIID(*objectFormat->pguid, GUID_POV)) return DIJOFS_POV(objectInstance);
			if (IsEqualIID(*objectFormat->pguid, GUID_Button)) return DIJOFS_BUTTON(objectInstance);
		}

		if (objectType & DIDFT_BUTTON)
		{
			return DIJOFS_BUTTON(objectInstance);
		}

		if (objectType & DIDFT_POV)
		{
			return DIJOFS_POV(objectInstance);
		}

		if (objectType & DIDFT_AXIS)
		{
			DWORD axisOffsets[] = { DIJOFS_X, DIJOFS_Y, DIJOFS_Z, DIJOFS_RX, DIJOFS_RY, DIJOFS_RZ };
			if (objectInstance < ARRAYSIZE(axisOffsets))
			{
				return axisOffsets[objectInstance];
			}
		}

		return 0xFFFFFFFF;
	}

	DWORD GetObjectDataSize(DWORD objectType)
	{
		if (objectType & DIDFT_BUTTON)
		{
			return sizeof(BYTE);
		}

		return sizeof(DWORD);
	}

	HRESULT CopyCustomDeviceState(DWORD cbData, LPVOID lpvData, const DIJOYSTATE2* sourceState)
	{
		if (dataFormat.rgodf == NULL || dataFormat.dwNumObjs == 0 || cbData < dataFormat.dwDataSize)
		{
			return DIERR_INVALIDPARAM;
		}

		ZeroMemory(lpvData, cbData);

		BYTE* destinationBytes = (BYTE*)lpvData;
		const BYTE* sourceBytes = (const BYTE*)sourceState;
		for (DWORD i = 0; i < dataFormat.dwNumObjs; i++)
		{
			const DIOBJECTDATAFORMAT* objectFormat = &dataFormat.rgodf[i];
			DWORD objectSize = GetObjectDataSize(objectFormat->dwType);
			if (objectFormat->dwOfs + objectSize > cbData)
			{
				continue;
			}

			DWORD sourceOffset = GetSourceOffsetForObject(objectFormat);
			if (sourceOffset == 0xFFFFFFFF || sourceOffset + objectSize > sizeof(DIJOYSTATE2))
			{
				continue;
			}

			memcpy(destinationBytes + objectFormat->dwOfs, sourceBytes + sourceOffset, objectSize);
		}

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_QueryInterface(GUID* riid, LPVOID* ppvObj)
	{
		diGlobalsInstance->LogA("GamepadDevice->QueryInterface()", __FILE__, __LINE__);

		return S_OK;
	};

	virtual ULONG __stdcall Base_AddRef()
	{
		diGlobalsInstance->LogA("GamepadDevice->AddRef()", __FILE__, __LINE__);
		refCount++;

		return refCount;
	}

	virtual ULONG __stdcall Base_Release()
	{
		diGlobalsInstance->LogA("GamepadDevice->Release()", __FILE__, __LINE__);

		if (refCount > 0)
		{
			refCount--;
		}

		if (refCount == 0)
		{
			ReleaseAllCreatedEffects();
			diGlobalsInstance->SetControllerVibration(xinputUserIndex, 0, 0);
		}

		return refCount;
	}


	virtual HRESULT STDMETHODCALLTYPE Base_GetCapabilities(LPDIDEVCAPS lpDIDevCaps) {
		diGlobalsInstance->LogA("GamepadDevice->GetCapabilities()", __FILE__, __LINE__);

		lpDIDevCaps->dwFlags = DIDC_ATTACHED | DIDC_EMULATED | DIDC_FORCEFEEDBACK;
		lpDIDevCaps->dwDevType = this->dwDevType;
		lpDIDevCaps->dwAxes = 6;
		lpDIDevCaps->dwButtons = 10;
		lpDIDevCaps->dwPOVs = 1;
		lpDIDevCaps->dwFFSamplePeriod = 20000;
		lpDIDevCaps->dwFFMinTimeResolution = 20000;
		lpDIDevCaps->dwFirmwareRevision = 0;
		lpDIDevCaps->dwHardwareRevision = 0;
		lpDIDevCaps->dwFFDriverVersion = 0;

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_GetProperty(GUID* rguidProp, LPDIPROPHEADER pdiph) {
		diGlobalsInstance->LogA("GamepadDevice->GetProperty()", __FILE__, __LINE__);

		return DIERR_UNSUPPORTED;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_SetProperty(GUID* rguidProp, LPCDIPROPHEADER pdiph) {
		diGlobalsInstance->LogA("GamepadDevice->SetProperty()", __FILE__, __LINE__);

		if (!rguidProp || !pdiph)
		{
			return DIERR_INVALIDPARAM;
		}

		return DIERR_UNSUPPORTED;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_Acquire() {
		diGlobalsInstance->LogA("GamepadDevice->Acquire()", __FILE__, __LINE__);		

		this->isAcquired = true;

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_Unacquire() {
		diGlobalsInstance->LogA("GamepadDevice->Unacquire()", __FILE__, __LINE__);

		StopAllCreatedEffects(false);
		diGlobalsInstance->SetControllerVibration(xinputUserIndex, 0, 0);
		this->isAcquired = false;

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_GetDeviceState(DWORD cbData, LPVOID lpvData) {
		diGlobalsInstance->LogA("GamepadDevice->GetDeviceState()", __FILE__, __LINE__);

		if (!this->isAcquired)
		{
			return DIERR_INPUTLOST;
		}

		if (!diGlobalsInstance->IsXInputControllerConnected(xinputUserIndex))
		{
			return DIERR_NOTATTACHED;
		}

		HRESULT result = DI_OK;
		diGlobalsInstance->Lock();
		{
			diGlobalsInstance->PopulateJoystickStateFromXInput(xinputUserIndex, diGlobalsInstance->gamepadState);

			if (cbData == sizeof(DIJOYSTATE))
			{
				memcpy(lpvData, diGlobalsInstance->gamepadState, sizeof(DIJOYSTATE));
			}
			else if (cbData == sizeof(DIJOYSTATE2))
			{
				memcpy(lpvData, diGlobalsInstance->gamepadState, sizeof(DIJOYSTATE2));
			}
			else if (dataFormat.dwDataSize > 0 && cbData >= dataFormat.dwDataSize)
			{
				result = CopyCustomDeviceState(cbData, lpvData, diGlobalsInstance->gamepadState);
			}
			else {
				diGlobalsInstance->LogA("GamepadDevice->GetDeviceState(): Unsupported cbData: %i", __FILE__, __LINE__,cbData);
				result = DIERR_UNSUPPORTED;
			}
		}
		diGlobalsInstance->Unlock();

		return result;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) {
		diGlobalsInstance->LogA("GamepadDevice->GetDeviceData()", __FILE__, __LINE__);

		*pdwInOut = 0;

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_SetDataFormat(LPCDIDATAFORMAT lpdf)
	{
		diGlobalsInstance->LogA("GamepadDevice->SetDataFormat()", __FILE__, __LINE__);
		if (!lpdf)
		{
			return DIERR_INVALIDPARAM;
		}

		memcpy(&dataFormat, lpdf, sizeof(DIDATAFORMAT));
		if (lpdf->dwNumObjs > 0)
		{
			if (!lpdf->rgodf || lpdf->dwNumObjs > ARRAYSIZE(objectDataFormat))
			{
				ZeroMemory(&dataFormat, sizeof(dataFormat));
				return DIERR_INVALIDPARAM;
			}

			memcpy(objectDataFormat, lpdf->rgodf, sizeof(DIOBJECTDATAFORMAT) * lpdf->dwNumObjs);
			dataFormat.rgodf = objectDataFormat;
		}

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_SetEventNotification(HANDLE hEvent)
	{
		diGlobalsInstance->LogA("GamepadDevice->SetEventNotification()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
	{
		diGlobalsInstance->LogA("GamepadDevice->SetCooperativeLevel(), dwFlags: %x", __FILE__, __LINE__, dwFlags);
		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_RunControlPanel(HWND hwndOwner, DWORD dwFlags)
	{
		diGlobalsInstance->LogA("GamepadDevice->RunControlPanel()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_Initialize(HINSTANCE hInst, DWORD dwVersion, GUID* rguid)
	{
		diGlobalsInstance->LogA("GamepadDevice->Initialize()", __FILE__, __LINE__);

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_CreateEffect(GUID* rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter)
	{
		diGlobalsInstance->LogA("GamepadDevice->CreateEffect()", __FILE__, __LINE__);

		if (!rguid || !ppdeff || punkOuter)
		{
			return DIERR_INVALIDPARAM;
		}

		*ppdeff = NULL;
		if (!IsSupportedEffectGuid(rguid))
		{
			return DIERR_UNSUPPORTED;
		}

		if (createdEffectCount >= ARRAYSIZE(createdEffects))
		{
			return DIERR_INVALIDPARAM;
		}

		CDirectInputEffectXInput* effect = new CDirectInputEffectXInput(xinputUserIndex, rguid);
		if (!effect)
		{
			return E_OUTOFMEMORY;
		}

		HRESULT hr = lpeff ? effect->SetParameters(lpeff, 0) : DI_OK;
		if (FAILED(hr))
		{
			effect->Release();
			return hr;
		}

		effect->AddRef();
		createdEffects[createdEffectCount++] = effect;
		*ppdeff = effect;
		return DI_OK;
	}


	virtual HRESULT STDMETHODCALLTYPE Base_GetForceFeedbackState(LPDWORD pdwOut)
	{
		diGlobalsInstance->LogA("GamepadDevice->GetForceFeedbackState()", __FILE__, __LINE__);

		if (!pdwOut)
		{
			return DIERR_INVALIDPARAM;
		}

		*pdwOut = 0;
		if (createdEffectCount == 0)
		{
			*pdwOut |= DIGFFS_EMPTY;
		}

		if (!forceFeedbackActuatorsEnabled)
		{
			*pdwOut |= DIGFFS_POWEROFF;
		}

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_SendForceFeedbackCommand(DWORD dwFlags)
	{
		diGlobalsInstance->LogA("GamepadDevice->SendForceFeedbackCommand()", __FILE__, __LINE__);

		switch (dwFlags)
		{
		case DISFFC_STOPALL:
			StopAllCreatedEffects(false);
			return diGlobalsInstance->SetControllerVibration(xinputUserIndex, 0, 0);
		case DISFFC_RESET:
			StopAllCreatedEffects(true);
			forceFeedbackPaused = false;
			forceFeedbackActuatorsEnabled = true;
			diGlobalsInstance->SetControllerForceFeedbackPaused(xinputUserIndex, false);
			diGlobalsInstance->SetControllerForceFeedbackActuatorsEnabled(xinputUserIndex, true);
			return diGlobalsInstance->SetControllerVibration(xinputUserIndex, 0, 0);
		case DISFFC_PAUSE:
			forceFeedbackPaused = true;
			diGlobalsInstance->SetControllerForceFeedbackPaused(xinputUserIndex, true);
			return diGlobalsInstance->SetControllerVibration(xinputUserIndex, 0, 0);
		case DISFFC_CONTINUE:
			forceFeedbackPaused = false;
			diGlobalsInstance->SetControllerForceFeedbackPaused(xinputUserIndex, false);
			diGlobalsInstance->RecomputeAndApplyRumble(xinputUserIndex);
			return DI_OK;
		case DISFFC_SETACTUATORSOFF:
			forceFeedbackActuatorsEnabled = false;
			diGlobalsInstance->SetControllerForceFeedbackActuatorsEnabled(xinputUserIndex, false);
			return diGlobalsInstance->SetControllerVibration(xinputUserIndex, 0, 0);
		case DISFFC_SETACTUATORSON:
			forceFeedbackActuatorsEnabled = true;
			diGlobalsInstance->SetControllerForceFeedbackActuatorsEnabled(xinputUserIndex, true);
			diGlobalsInstance->RecomputeAndApplyRumble(xinputUserIndex);
			return DI_OK;
		default:
			return DIERR_UNSUPPORTED;
		}
	}

	virtual HRESULT STDMETHODCALLTYPE Base_EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl)
	{
		diGlobalsInstance->LogA("GamepadDevice->EnumCreatedEffectObjects()", __FILE__, __LINE__);

		if (!lpCallback)
		{
			return DIERR_INVALIDPARAM;
		}

		for (DWORD i = 0; i < createdEffectCount; i++)
		{
			if (createdEffects[i] && lpCallback(createdEffects[i], pvRef) == DIENUM_STOP)
			{
				break;
			}
		}

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_Escape(LPDIEFFESCAPE pesc)
	{
		diGlobalsInstance->LogA("GamepadDevice->Escape()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_Poll() {
		//diGlobalsInstance->LogA("GamepadDevice->Poll()", __FILE__, __LINE__);

		return DI_NOEFFECT;
	}

	virtual HRESULT STDMETHODCALLTYPE Base_SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl) {
		diGlobalsInstance->LogA("GamepadDevice->SendDeviceData()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}

	void StopAllCreatedEffects(bool unload)
	{
		for (DWORD i = 0; i < createdEffectCount; i++)
		{
			if (!createdEffects[i])
			{
				continue;
			}

			if (unload)
			{
				createdEffects[i]->Unload();
			}
			else
			{
				createdEffects[i]->Stop();
			}
		}
	}

	void ReleaseAllCreatedEffects()
	{
		for (DWORD i = 0; i < createdEffectCount; i++)
		{
			if (!createdEffects[i])
			{
				continue;
			}

			createdEffects[i]->Unload();
			createdEffects[i]->Release();
			createdEffects[i] = NULL;
		}

		createdEffectCount = 0;
	}
};

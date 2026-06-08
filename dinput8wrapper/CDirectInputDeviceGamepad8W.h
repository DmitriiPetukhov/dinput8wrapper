#pragma once

class CDirectInputDeviceGamepad8W : public CDirectInputDeviceGamepad8, public IDirectInputDevice8W
{
private:
	DIDEVICEINSTANCEW* gamepadDeviceInfo;

public:

	CDirectInputDeviceGamepad8W(DWORD userIndex) : CDirectInputDeviceGamepad8(userIndex)
	{
		gamepadDeviceInfo = new DIDEVICEINSTANCEW();
		ZeroMemory(gamepadDeviceInfo, sizeof(DIDEVICEINSTANCEW));
		gamepadDeviceInfo->dwSize = sizeof(DIDEVICEINSTANCEW);
		gamepadDeviceInfo->guidInstance = diGlobalsInstance->gamepadInstanceGuids[userIndex];
		gamepadDeviceInfo->guidProduct = GUID_Xbox360Controller;
		gamepadDeviceInfo->dwDevType = DIDEVTYPE_HID | DI8DEVTYPE_GAMEPAD | (DI8DEVTYPEGAMEPAD_STANDARD << 8);
		gamepadDeviceInfo->wUsage = HID_USAGE_GENERIC_GAMEPAD;
		gamepadDeviceInfo->wUsagePage = HID_USAGE_PAGE_GENERIC;
		StringCbPrintfW(gamepadDeviceInfo->tszInstanceName, 260, L"Controller (Gamepad XBox360) %lu", userIndex + 1);
		StringCbCopyW(gamepadDeviceInfo->tszProductName, 260, L"Controller (Gamepad XBox360)");

		this->dwDevType = gamepadDeviceInfo->dwDevType;
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(GUID* riid, LPVOID* ppvObj)
	{
		return Base_QueryInterface(riid, ppvObj);
	};

	virtual ULONG __stdcall AddRef()
	{
		return Base_AddRef();
	}

	virtual ULONG __stdcall Release()
	{
		return Base_Release();
	}


	virtual HRESULT STDMETHODCALLTYPE GetCapabilities(LPDIDEVCAPS lpDIDevCaps) {
		return Base_GetCapabilities(lpDIDevCaps);
	}

	virtual HRESULT STDMETHODCALLTYPE GetProperty(GUID* rguidProp, LPDIPROPHEADER pdiph) {
		return Base_GetProperty(rguidProp, pdiph);
	}

	virtual HRESULT STDMETHODCALLTYPE SetProperty(GUID* rguidProp, LPCDIPROPHEADER pdiph) {
		return Base_SetProperty(rguidProp, pdiph);
	}

	virtual HRESULT STDMETHODCALLTYPE Acquire() {
		return Base_Acquire();
	}

	virtual HRESULT STDMETHODCALLTYPE Unacquire() {
		return Base_Unacquire();
	}

	virtual HRESULT STDMETHODCALLTYPE GetDeviceState(DWORD cbData, LPVOID lpvData) {
		return Base_GetDeviceState(cbData, lpvData);
	}

	virtual HRESULT STDMETHODCALLTYPE GetDeviceData(DWORD cbObjectData, LPDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD dwFlags) {
		return Base_GetDeviceData(cbObjectData, rgdod, pdwInOut, dwFlags);
	}

	virtual HRESULT STDMETHODCALLTYPE SetDataFormat(LPCDIDATAFORMAT lpdf)
	{
		return Base_SetDataFormat(lpdf);
	}

	virtual HRESULT STDMETHODCALLTYPE SetEventNotification(HANDLE hEvent)
	{
		return Base_SetEventNotification(hEvent);
	}

	virtual HRESULT STDMETHODCALLTYPE SetCooperativeLevel(HWND hwnd, DWORD dwFlags)
	{
		return Base_SetCooperativeLevel(hwnd, dwFlags);
	}

	virtual HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags)
	{
		return Base_RunControlPanel(hwndOwner, dwFlags);
	}

	virtual HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hInst, DWORD dwVersion, GUID* rguid)
	{
		return Base_Initialize(hInst, dwVersion, rguid);
	}

	virtual HRESULT STDMETHODCALLTYPE CreateEffect(GUID* rguid, LPCDIEFFECT lpeff, LPDIRECTINPUTEFFECT* ppdeff, LPUNKNOWN punkOuter)
	{
		return Base_CreateEffect(rguid, lpeff, ppdeff, punkOuter);
	}

	virtual HRESULT STDMETHODCALLTYPE GetForceFeedbackState(LPDWORD pdwOut)
	{
		return Base_GetForceFeedbackState(pdwOut);
	}

	virtual HRESULT STDMETHODCALLTYPE SendForceFeedbackCommand(DWORD dwFlags)
	{
		return Base_SendForceFeedbackCommand(dwFlags);
	}

	virtual HRESULT STDMETHODCALLTYPE EnumCreatedEffectObjects(LPDIENUMCREATEDEFFECTOBJECTSCALLBACK lpCallback, LPVOID pvRef, DWORD fl)
	{
		return Base_EnumCreatedEffectObjects(lpCallback, pvRef, fl);
	}

	virtual HRESULT STDMETHODCALLTYPE Escape(LPDIEFFESCAPE pesc)
	{
		return Base_Escape(pesc);
	}

	virtual HRESULT STDMETHODCALLTYPE Poll() {
		return Base_Poll();
	}

	virtual HRESULT STDMETHODCALLTYPE SendDeviceData(DWORD cbObjectData, LPCDIDEVICEOBJECTDATA rgdod, LPDWORD pdwInOut, DWORD fl) {
		return Base_SendDeviceData(cbObjectData, rgdod, pdwInOut, fl);
	}





	virtual HRESULT STDMETHODCALLTYPE EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags) {

		diGlobalsInstance->LogA("GamepadDevice->EnumObjects(), dwFlags: %x", __FILE__, __LINE__, dwFlags);

		struct AxisInfo { const GUID* guid; DWORD offset; const wchar_t* name; };
		AxisInfo axes[] = {
			{ &GUID_XAxis, DIJOFS_X, L"X-Axis" },
			{ &GUID_YAxis, DIJOFS_Y, L"Y-Axis" },
			{ &GUID_ZAxis, DIJOFS_Z, L"Left Trigger" },
			{ &GUID_RxAxis, DIJOFS_RX, L"Rx-Axis" },
			{ &GUID_RyAxis, DIJOFS_RY, L"Ry-Axis" },
			{ &GUID_RzAxis, DIJOFS_RZ, L"Right Trigger" },
		};

		for (int i = 0; i < ARRAYSIZE(axes); i++)
		{
			DWORD type = DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(i);
			if (!ShouldEnumObject(dwFlags, type))
			{
				continue;
			}

			DIDEVICEOBJECTINSTANCEW objectInfo = {};
			objectInfo.dwSize = sizeof(DIDEVICEOBJECTINSTANCEW);
			objectInfo.guidType = *axes[i].guid;
			objectInfo.dwOfs = axes[i].offset;
			objectInfo.dwType = type;
			StringCbCopyW(objectInfo.tszName, MAX_PATH, axes[i].name);
			if (lpCallback(&objectInfo, pvRef) == DIENUM_STOP)
			{
				return DI_OK;
			}
		}

		if (ShouldEnumObject(dwFlags, DIDFT_POV))
		{
			DIDEVICEOBJECTINSTANCEW objectInfo = {};
			objectInfo.dwSize = sizeof(DIDEVICEOBJECTINSTANCEW);
			objectInfo.guidType = GUID_POV;
			objectInfo.dwOfs = DIJOFS_POV(0);
			objectInfo.dwType = DIDFT_POV;
			StringCbCopyW(objectInfo.tszName, MAX_PATH, L"POV");
			if (lpCallback(&objectInfo, pvRef) == DIENUM_STOP)
			{
				return DI_OK;
			}
		}

		for (int i = 0; i < 10; i++)
		{
			DWORD type = DIDFT_PSHBUTTON | DIDFT_MAKEINSTANCE(i);
			if (!ShouldEnumObject(dwFlags, type))
			{
				continue;
			}

			DIDEVICEOBJECTINSTANCEW objectInfo = {};
			objectInfo.dwSize = sizeof(DIDEVICEOBJECTINSTANCEW);
			objectInfo.guidType = GUID_Button;
			objectInfo.dwOfs = DIJOFS_BUTTON(i);
			objectInfo.dwType = type;
			StringCbPrintfW(objectInfo.tszName, MAX_PATH, L"Button %i", i + 1);
			if (lpCallback(&objectInfo, pvRef) == DIENUM_STOP)
			{
				return DI_OK;
			}
		}

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE GetObjectInfo(LPDIDEVICEOBJECTINSTANCEW pdidoi, DWORD dwObj, DWORD dwHow)
	{
		diGlobalsInstance->LogA("GamepadDevice->GetObjectInfo()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCEW pdidi)
	{
		diGlobalsInstance->LogA("GamepadDevice->GetDeviceInfo()", __FILE__, __LINE__);
		memcpy(pdidi, gamepadDeviceInfo, sizeof(DIDEVICEINSTANCEW));

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE EnumEffects(LPDIENUMEFFECTSCALLBACKW lpCallback, LPVOID pvRef, DWORD dwEffType)
	{
		diGlobalsInstance->LogA("GamepadDevice->EnumEffects()", __FILE__, __LINE__);

		if (!lpCallback)
		{
			return DIERR_INVALIDPARAM;
		}

		if (dwEffType == 0 || DIEFT_GETTYPE(dwEffType) == DIEFT_CONSTANTFORCE)
		{
			DIEFFECTINFOW info = {};
			info.dwSize = sizeof(DIEFFECTINFOW);
			info.guid = GUID_ConstantForce;
			info.dwEffType = DIEFT_CONSTANTFORCE;
			info.dwStaticParams = DIEP_TYPESPECIFICPARAMS;
			info.dwDynamicParams = DIEP_GAIN | DIEP_DURATION;
			StringCbCopyW(info.tszName, MAX_PATH, L"XInput Constant Force");

			lpCallback(&info, pvRef);
		}

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE GetEffectInfo(LPDIEFFECTINFOW pdei, GUID* rguid)
	{
		diGlobalsInstance->LogA("GamepadDevice->GetEffectInfo()", __FILE__, __LINE__);

		if (!pdei || !rguid || pdei->dwSize < sizeof(DIEFFECTINFOW))
		{
			return DIERR_INVALIDPARAM;
		}

		if (!IsEqualIID(*rguid, GUID_ConstantForce))
		{
			return DIERR_UNSUPPORTED;
		}

		ZeroMemory(pdei, sizeof(DIEFFECTINFOW));
		pdei->dwSize = sizeof(DIEFFECTINFOW);
		pdei->guid = GUID_ConstantForce;
		pdei->dwEffType = DIEFT_CONSTANTFORCE;
		pdei->dwStaticParams = DIEP_TYPESPECIFICPARAMS;
		pdei->dwDynamicParams = DIEP_GAIN | DIEP_DURATION;
		StringCbCopyW(pdei->tszName, MAX_PATH, L"XInput Constant Force");

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE EnumEffectsInFile(LPCWSTR lpszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags)
	{
		diGlobalsInstance->LogA("GamepadDevice->EnumEffectsInFile()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE WriteEffectToFile(LPCWSTR lpszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags)
	{
		diGlobalsInstance->LogA("GamepadDevice->WriteEffectToFile()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE BuildActionMap(LPDIACTIONFORMATW lpdiaf, LPCWSTR lpszUserName, DWORD dwFlags)
	{
		diGlobalsInstance->LogA("GamepadDevice->BuildActionMap()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE SetActionMap(LPDIACTIONFORMATW lpdiaf, LPCWSTR lpszUserName, DWORD dwFlags)
	{
		diGlobalsInstance->LogA("GamepadDevice->SetActionMap()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetImageInfo(LPDIDEVICEIMAGEINFOHEADERW lpdiDevImageInfoHeader)
	{
		diGlobalsInstance->LogA("GamepadDevice->GetImageInfo()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}
};

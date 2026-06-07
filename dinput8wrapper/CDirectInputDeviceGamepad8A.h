#pragma once

class CDirectInputDeviceGamepad8A : public CDirectInputDeviceGamepad8, public IDirectInputDevice8A
{
private:
	DIDEVICEINSTANCEA* gamepadDeviceInfo;

public:

	CDirectInputDeviceGamepad8A(DWORD userIndex) : CDirectInputDeviceGamepad8(userIndex)
	{
		gamepadDeviceInfo = new DIDEVICEINSTANCEA();
		ZeroMemory(gamepadDeviceInfo, sizeof(DIDEVICEINSTANCEA));
		gamepadDeviceInfo->dwSize = sizeof(DIDEVICEINSTANCEA);
		gamepadDeviceInfo->guidInstance = diGlobalsInstance->gamepadInstanceGuids[userIndex];
		gamepadDeviceInfo->guidProduct = GUID_Xbox360Controller;
		gamepadDeviceInfo->dwDevType = DIDEVTYPE_HID | DI8DEVTYPE_GAMEPAD | (DI8DEVTYPEGAMEPAD_STANDARD << 8);
		gamepadDeviceInfo->wUsage = HID_USAGE_GENERIC_GAMEPAD;
		gamepadDeviceInfo->wUsagePage = HID_USAGE_PAGE_GENERIC;
		StringCbPrintfA(gamepadDeviceInfo->tszInstanceName, 260, "Controller (Gamepad XBox360) %lu", userIndex + 1);
		StringCbCopyA(gamepadDeviceInfo->tszProductName, 260, "Controller (Gamepad XBox360)");

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
		return Base_CreateEffect(rguid, lpeff,  ppdeff, punkOuter);
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



























	virtual HRESULT STDMETHODCALLTYPE EnumObjects(LPDIENUMDEVICEOBJECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwFlags) {

		diGlobalsInstance->LogA("GamepadDevice->EnumObjects(), dwFlags: %x", __FILE__, __LINE__, dwFlags);

		struct AxisInfo { const GUID* guid; DWORD offset; const char* name; };
		AxisInfo axes[] = {
			{ &GUID_XAxis, DIJOFS_X, "X-Axis" },
			{ &GUID_YAxis, DIJOFS_Y, "Y-Axis" },
			{ &GUID_ZAxis, DIJOFS_Z, "Left Trigger" },
			{ &GUID_RxAxis, DIJOFS_RX, "Rx-Axis" },
			{ &GUID_RyAxis, DIJOFS_RY, "Ry-Axis" },
			{ &GUID_RzAxis, DIJOFS_RZ, "Right Trigger" },
		};

		for (int i = 0; i < ARRAYSIZE(axes); i++)
		{
			DWORD type = DIDFT_ABSAXIS | DIDFT_MAKEINSTANCE(i);
			if (!ShouldEnumObject(dwFlags, type))
			{
				continue;
			}

			DIDEVICEOBJECTINSTANCEA objectInfo = {};
			objectInfo.dwSize = sizeof(DIDEVICEOBJECTINSTANCEA);
			objectInfo.guidType = *axes[i].guid;
			objectInfo.dwOfs = axes[i].offset;
			objectInfo.dwType = type;
			StringCbCopyA(objectInfo.tszName, MAX_PATH, axes[i].name);
			if (lpCallback(&objectInfo, pvRef) == DIENUM_STOP)
			{
				return DI_OK;
			}
		}

		if (ShouldEnumObject(dwFlags, DIDFT_POV))
		{
			DIDEVICEOBJECTINSTANCEA objectInfo = {};
			objectInfo.dwSize = sizeof(DIDEVICEOBJECTINSTANCEA);
			objectInfo.guidType = GUID_POV;
			objectInfo.dwOfs = DIJOFS_POV(0);
			objectInfo.dwType = DIDFT_POV;
			StringCbCopyA(objectInfo.tszName, MAX_PATH, "POV");
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

			DIDEVICEOBJECTINSTANCEA objectInfo = {};
			objectInfo.dwSize = sizeof(DIDEVICEOBJECTINSTANCEA);
			objectInfo.guidType = GUID_Button;
			objectInfo.dwOfs = DIJOFS_BUTTON(i);
			objectInfo.dwType = type;
			StringCbPrintfA(objectInfo.tszName, MAX_PATH, "Button %i", i + 1);
			if (lpCallback(&objectInfo, pvRef) == DIENUM_STOP)
			{
				return DI_OK;
			}
		}

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE GetObjectInfo(LPDIDEVICEOBJECTINSTANCEA pdidoi, DWORD dwObj, DWORD dwHow)
	{
		diGlobalsInstance->LogA("GamepadDevice->GetObjectInfo()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetDeviceInfo(LPDIDEVICEINSTANCEA pdidi)
	{
		diGlobalsInstance->LogA("GamepadDevice->GetDeviceInfo()", __FILE__, __LINE__);
		memcpy(pdidi, gamepadDeviceInfo, sizeof(DIDEVICEINSTANCEA));

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE EnumEffects(LPDIENUMEFFECTSCALLBACKA lpCallback, LPVOID pvRef, DWORD dwEffType)
	{
		diGlobalsInstance->LogA("GamepadDevice->EnumEffects()", __FILE__, __LINE__);

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE GetEffectInfo(LPDIEFFECTINFOA pdei, GUID* rguid)
	{
		diGlobalsInstance->LogA("GamepadDevice->GetEffectInfo()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE EnumEffectsInFile(LPCSTR lpszFileName, LPDIENUMEFFECTSINFILECALLBACK pec, LPVOID pvRef, DWORD dwFlags)
	{
		diGlobalsInstance->LogA("GamepadDevice->EnumEffectsInFile()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE WriteEffectToFile(LPCSTR lpszFileName, DWORD dwEntries, LPDIFILEEFFECT rgDiFileEft, DWORD dwFlags)
	{
		diGlobalsInstance->LogA("GamepadDevice->WriteEffectToFile()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE BuildActionMap(LPDIACTIONFORMATA lpdiaf, LPCSTR lpszUserName, DWORD dwFlags)
	{
		diGlobalsInstance->LogA("GamepadDevice->BuildActionMap()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE SetActionMap(LPDIACTIONFORMATA lpdiaf, LPCSTR lpszUserName, DWORD dwFlags)
	{
		diGlobalsInstance->LogA("GamepadDevice->SetActionMap()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}

	virtual HRESULT STDMETHODCALLTYPE GetImageInfo(LPDIDEVICEIMAGEINFOHEADERA lpdiDevImageInfoHeader)
	{
		diGlobalsInstance->LogA("GamepadDevice->GetImageInfo()", __FILE__, __LINE__);

		return E_NOTIMPL;
	}
};

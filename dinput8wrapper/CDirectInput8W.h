#pragma once

class CDirectInput8W : public IDirectInput8W, public CDirectInput8
{
private:	
	CDirectInputDeviceMouse8W* mouseDevice;
	CDirectInputDeviceKeyboard8W* keyboardDevice;
	CDirectInputDeviceGamepad8W* gamepadDevices[4];

	DIDEVICEINSTANCEW mouseDeviceInfo = {};
	DIDEVICEINSTANCEW keyboardDeviceInfo = {};
	DIDEVICEINSTANCEW gamepadDeviceInfo = {};

public:


	CDirectInput8W() : CDirectInput8()
	{		
		mouseDevice = new CDirectInputDeviceMouse8W();
		keyboardDevice = new CDirectInputDeviceKeyboard8W();
		for (DWORD i = 0; i < 4; i++)
		{
			gamepadDevices[i] = new CDirectInputDeviceGamepad8W(i);
		}
	}

	virtual HRESULT STDMETHODCALLTYPE QueryInterface(GUID* riid, LPVOID* ppvObj)
	{
		return Base_QueryInterface(riid, ppvObj);
	}

	virtual ULONG __stdcall AddRef()
	{
		return Base_AddRef();
	}

	virtual ULONG __stdcall Release()
	{
		return Base_Release();
	}

	virtual HRESULT STDMETHODCALLTYPE GetDeviceStatus(GUID* rguidInstance)
	{
		return Base_GetDeviceStatus(rguidInstance);
	}

	virtual HRESULT STDMETHODCALLTYPE RunControlPanel(HWND hwndOwner, DWORD dwFlags)
	{
		return Base_RunControlPanel(hwndOwner, dwFlags);
	}

	virtual HRESULT STDMETHODCALLTYPE Initialize(HINSTANCE hinst, DWORD dwVersion)
	{
		return Base_Initialize(hinst, dwVersion);
	}

	/*** IDirectInput8W methods ***/
	virtual HRESULT STDMETHODCALLTYPE CreateDevice(GUID* rguid, LPDIRECTINPUTDEVICE8W* lplpDirectInputDevice, LPUNKNOWN pUnkOuter)
	{
		if (!lplpDirectInputDevice)
		{
			return DIERR_INVALIDPARAM;
		}

		*lplpDirectInputDevice = nullptr;

		if (!rguid)
		{
			return DIERR_INVALIDPARAM;
		}

		if (IsEqualIID(GUID_SysMouse, *rguid))
		{
			diGlobalsInstance->LogA("CreateDevice() for GUID_SysMouse", __FILE__, __LINE__);

			*lplpDirectInputDevice = mouseDevice;
			return DI_OK;
		}
		else if (IsEqualIID(GUID_SysKeyboard, *rguid))
		{
			diGlobalsInstance->LogA("CreateDevice() for GUID_SysKeyboard", __FILE__, __LINE__);

			*lplpDirectInputDevice = keyboardDevice;
			return DI_OK;
		}

		DWORD gamepadIndex = diGlobalsInstance->GetXInputControllerIndex(rguid);
		if (gamepadIndex < 4 && diGlobalsInstance->IsXInputControllerConnected(gamepadIndex))
		{
			diGlobalsInstance->LogA("CreateDevice() for XInput gamepad %lu", __FILE__, __LINE__, gamepadIndex);

			*lplpDirectInputDevice = gamepadDevices[gamepadIndex];
			return DI_OK;
		}

		diGlobalsInstance->LogA("CreateDevice() rejecting unsupported rguid=%x-%x-%x-%x", __FILE__, __LINE__, rguid->Data1, rguid->Data2, rguid->Data3, rguid->Data4);

		return DIERR_DEVICENOTREG;
	}

	virtual HRESULT STDMETHODCALLTYPE EnumDevices(DWORD dwDevType, LPDIENUMDEVICESCALLBACKW lpCallback, LPVOID pvRef, DWORD dwFlags)
	{		
		diGlobalsInstance->LogA("EnumDevices() for dwDevType=0x%x, dwFlags=0x0", __FILE__, __LINE__, dwDevType, dwFlags);

		if ((dwDevType == DI8DEVCLASS_ALL) || (dwDevType == DI8DEVCLASS_POINTER) || (dwDevType == DI8DEVTYPE_MOUSE))
		{
			diGlobalsInstance->LogA("EnumDevices: Returning Mouse-Device", __FILE__, __LINE__);

			mouseDevice->GetDeviceInfo(&mouseDeviceInfo);
			if (lpCallback(&mouseDeviceInfo, pvRef) == DIENUM_STOP)
			{
				diGlobalsInstance->LogA("EnumDevices stopped (due to DIENUM_STOP)", __FILE__, __LINE__);
				return DI_OK;
			}
		}

		if ((dwDevType == DI8DEVCLASS_ALL) || (dwDevType == DI8DEVCLASS_KEYBOARD))
		{
			diGlobalsInstance->LogA("EnumDevices: Returning Keyboard-Device", __FILE__, __LINE__);

			keyboardDevice->GetDeviceInfo(&keyboardDeviceInfo);
			if (lpCallback(&keyboardDeviceInfo, pvRef) == DIENUM_STOP)
			{
				diGlobalsInstance->LogA("EnumDevices stopped (due to DIENUM_STOP)", __FILE__, __LINE__);
				return DI_OK;
			}
		}

		if ((dwDevType == DI8DEVCLASS_ALL) || (dwDevType == DI8DEVCLASS_GAMECTRL) || (dwDevType == DI8DEVTYPE_GAMEPAD) || (dwDevType == DI8DEVTYPE_JOYSTICK))
		{
			for (DWORD i = 0; i < 4; i++)
			{
				if (!diGlobalsInstance->IsXInputControllerConnected(i))
				{
					continue;
				}

				diGlobalsInstance->LogA("EnumDevices: Returning XInput Gamepad %lu", __FILE__, __LINE__, i);

				gamepadDevices[i]->GetDeviceInfo(&gamepadDeviceInfo);
				if (lpCallback(&gamepadDeviceInfo, pvRef) == DIENUM_STOP)
				{
					diGlobalsInstance->LogA("EnumDevices stopped (due to DIENUM_STOP)", __FILE__, __LINE__);
					return DI_OK;
				}
			}
		}

		return DI_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE FindDevice(GUID* rguid, LPCWSTR pszName, LPGUID pguidInstance)
	{
		MessageBoxA(NULL, "FindDevice", "input8.dll", MB_OK);

		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE EnumDevicesBySemantics(LPCWSTR ptszUserName, LPDIACTIONFORMATW lpdiActionFormat, LPDIENUMDEVICESBYSEMANTICSCBW lpCallback, LPVOID pvRef, DWORD dwFlags)
	{
		MessageBoxA(NULL, "EnumDevicesBySemantics", "input8.dll", MB_OK);

		return S_OK;
	}

	virtual HRESULT STDMETHODCALLTYPE ConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback, LPDICONFIGUREDEVICESPARAMSW lpdiCDParams, DWORD dwFlags, LPVOID pvRefData)
	{
		MessageBoxA(NULL, "ConfigureDevices", "input8.dll", MB_OK);

		return S_OK;
	}
};

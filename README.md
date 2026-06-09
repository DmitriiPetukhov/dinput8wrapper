# dinput8wrapper
This is a DirectInput 8 (dinput8.dll) Wrapper, which internally uses Raw-Input instead of DirectInput.

Initially created to have a different way to fix the frame stuttering caused by regular EnumDevices()-calls in "Kane & Lynch: Dead Men"
Other workarounds are listed here: https://www.pcgamingwiki.com/wiki/Kane_%26_Lynch:_Dead_Men#Stuttering

### Whats the issue?
Dependending on the number and type of physical/virtual "USB HID"-devices on the system the EnumDevices()-call can take a few ms to return. Games sometimes call this function regularly in the main thread to check if a new gamepad is attached/detached, which will then cause regular frame stuttering due to the pauses caused by EnumDevices().
This wrapper returns a fixed keyboard-device and a fixed mouse-device in EnumDevices() and does not do any real device enumeration.

## Current state:
* Supports keyboard and mouse through Raw Input.
* Gamepad support exposes XInput controllers as DirectInput gamepads.
* Force feedback for XInput gamepads is emulated through XInput rumble.
* Does only implement a small subset of the DirectInput8-Interfaces (may not work with all games using DirectInput8)
* 32-bit only (Is there any DirectInput8-game that requires x64?)

### XInput force-feedback emulation

Supported DirectInput effects:

* ConstantForce
* Sine, Square, Triangle, SawtoothUp, SawtoothDown
* RampForce

Unsupported DirectInput force-feedback fields such as direction vectors,
envelopes, triggers, sample periods, and start delays are accepted when valid
and safely stored or ignored where XInput has no matching behavior.

Errors are reserved for invalid parameters, invalid memory or pointer cases,
unsupported effect families, disconnected controllers, and operations that
would be unsafe to approximate.

DirectInput force-feedback output is reduced to XInput left and right motor
rumble. True directional forces, custom force fields, and actuator-specific
hardware behavior cannot be physically emulated.

## How to use / Installation
* Copy the dinput8.dll (see  https://github.com/geeky/dinput8wrapper/releases) to the folder where your game has its main executable 
* Hope that the game will load it instead of the original dinput8.dll from the windows-system32-folder \
  (Its a bad idea to override the original dinput8.dll!)

## Compile yourself
* This repository includes a Visual Studio Solution.
* Currently compiled with VS2022 Community Edition \
  https://visualstudio.microsoft.com/vs/preview/vs2022/ \
  "Desktop development with C++" should be selected in the installer

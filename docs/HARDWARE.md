# Hardware notes

Target: M5Stack Cardputer Adv.

The initial build targets the ESP32-S3 Arduino platform through PlatformIO and the M5Cardputer library. Board configuration and library compatibility must be confirmed by the cloud build and then by physical-device testing.

## Expected built-in capabilities

- Integrated display and keyboard
- 2.4 GHz Wi-Fi
- Bluetooth Low Energy
- microSD slot
- Infrared emitter and onboard audio hardware

## Important limits

- This is an ESP32-S3 firmware target, not a Linux system.
- The built-in radio and driver stack do not provide every capability of a dedicated monitor-mode adapter or a Kali Linux workstation.
- Wi-Fi and BLE concurrency can be constrained; scan operations should be bounded and tested.
- GATT access depends on the peripheral, permissions, pairing state and protocol.
- microSD operations must handle an absent or damaged card without crashing.
- No NFC or sub-GHz radio is assumed.
- Features are not considered hardware-verified until tested on a physical Cardputer Adv.

## Safety scope

HELLEBORE is intended for authorized assessment and learning. Implement passive discovery, non-disruptive diagnostics, controlled simulations and defensive detection exercises. Do not implement jamming, disruptive deauthentication, credential capture, or unauthorized access.

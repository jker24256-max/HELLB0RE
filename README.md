# HELLEBORE

**A dashboard-first wireless reconnaissance and defensive-assessment toolkit for the M5Stack Cardputer Adv.**

> Repository name: `HELLB0RE` (zero in the repository name). Product name: **HELLEBORE**.

NIGHTSHADE is a separate project and is intentionally untouched.

## Project principles

- Implement real, testable functions rather than decorative menu entries.
- Keep the dashboard keyboard-driven and responsive.
- Store reports on microSD when available.
- Use wireless assessment only on devices and networks you own or are explicitly authorized to test.
- Report capability honestly: a cloud build is not hardware validation.

## Current capabilities

- Dashboard, terminal, device diagnostics, and microSD report export.
- Wi-Fi access-point discovery: SSID, BSSID, RSSI, channel, and open/secured classification.
- BLE advertisement discovery and device signal-strength inventory.
- Wireless Lab audits based on real scan results:
  - open-access-point exposure count;
  - duplicate SSID / distinct-BSSID clues (a heuristic, not proof of a rogue AP);
  - a channel occupancy snapshot from discovered APs (not a spectrum analyzer);
  - BLE advertisement inventory.
- Terminal commands: `help`, `about`, `status`, `clear`, `scanwifi`, `scanble`, `report`.

## Explicit limitations

This is not a full Kali Linux replacement. The Cardputer Adv's ESP32-S3 is a 2.4 GHz Wi-Fi and BLE platform with constrained memory and radio capabilities. This firmware does not implement Wi-Fi jamming, disruptive deauthentication, credential capture, fake login portals, password cracking, or unauthorized exploitation. The current Wireless Lab provides passive discovery and non-disruptive defensive checks, not an offensive attack suite.

## Build

Install PlatformIO Core, then run:

```sh
pio run -e cardputer_adv
```

The firmware binary is normally written to `.pio/build/cardputer_adv/firmware.bin`. GitHub Actions compiles the firmware, creates a merged flash image, and uploads artifacts when the workflow succeeds.

## Controls

- Press `1`–`6` on the dashboard to open a module.
- Use `W` / `S` to navigate and Enter to open.
- In Wi-Fi or BLE discovery, press `R` to scan and `W` / `S` to scroll.
- In Wireless Lab, press `1` for Wi-Fi exposure, `2` for duplicate-SSID clues, `3` for channel occupancy, or `4` for BLE inventory.
- Press `E` to save a report where supported; press Esc or Backspace to return.
- In the terminal, type a command and press Enter.

## Validation status

A successful CI compile verifies that the firmware builds, not that it behaves correctly on the physical Cardputer Adv. SD-card pin mapping and all runtime behavior still require device testing.

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md), [docs/HARDWARE.md](docs/HARDWARE.md), and [docs/FLASHING.md](docs/FLASHING.md).

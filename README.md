# HELLEBORE

**A dashboard-first wireless diagnostics and security-learning platform for the M5Stack Cardputer Adv.**

> Repository name: `HELLB0RE` (zero in the repository name). Product name: **HELLEBORE**.

HELLEBORE is being built from a clean foundation. NIGHTSHADE is a separate project and is intentionally untouched.

## Project principles

- Build small, testable modules instead of shipping nonfunctional menu entries.
- Keep the dashboard responsive and keyboard-driven.
- Report capability honestly: a cloud build does not prove hardware behavior.
- Store reports on microSD when available, with graceful failure when it is not.
- Support authorized defensive assessment and controlled simulations. No jamming, disruptive deauthentication, credential theft, or rogue credential-capture portals.

## Current milestone: v0.1 foundation

- [x] Repository architecture and cloud-build workflow
- [x] Dashboard and keyboard input foundation
- [x] Minimal terminal commands: `help`, `about`, `status`, `clear`
- [x] Host-side command parser tests
- [x] GitHub Actions firmware artifact
- [ ] Verify build in GitHub Actions
- [ ] Verify screen and keyboard behavior on a physical Cardputer Adv

The first build uses PlatformIO with the Arduino framework, ESP32-S3, and M5Cardputer library.

## Build locally

Install PlatformIO Core, then run:

```sh
pio run
```

The firmware binary is normally written to `.pio/build/cardputer_adv/firmware.bin`.

## Cloud builds

Push to `main` or open a pull request. GitHub Actions runs host-side parser tests, compiles the firmware, and uploads the build output as an artifact. You can also start the workflow manually from the Actions tab.

## Controls

- Type `1`–`4` to select a dashboard module.
- Press Enter to open the selected module.
- Press `t` to open the terminal.
- Press Esc or Backspace to return to the dashboard.
- In the terminal, type a command and press Enter.

Available terminal commands: `help`, `about`, `status`, `clear`.

## Roadmap

1. Minimal, tested firmware foundation.
2. Wi-Fi discovery and non-disruptive configuration assessment.
3. BLE discovery and authorized GATT inspection.
4. microSD reporting and diagnostics.
5. IR tools and additional hardware diagnostics.
6. Controlled attack simulations and detection exercises.
7. Cross-radio event timeline, scoring, and evidence export.

See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) and [docs/HARDWARE.md](docs/HARDWARE.md).

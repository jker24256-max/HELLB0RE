# Flashing HELLEBORE

## Goal

Use one consolidated release image for a clean installation, rather than flashing a separate firmware image for each module. New firmware features added later still require a firmware update; no firmware can gain code that was not included when it was flashed.

## Download

1. Open the repository's **Actions** tab.
2. Open the latest successful **HELLEBORE CI** run.
3. Download the `hellebore-cardputer-adv-firmware` artifact.
4. Prefer the merged full-flash image when it is included in the artifact. Keep the individual binaries as recovery/debugging files.

## Flashing notes

- Confirm the target is an **M5Stack Cardputer Adv** before writing firmware.
- Use a trusted ESP32-S3 flashing utility compatible with a merged ESP image, or use esptool with the exact offsets documented by the build output.
- A clean installation can erase existing firmware and settings. Back up anything you need first.
- Do not disconnect USB or power during flashing.
- If the image does not boot, use the individual bootloader/partition/application images and the offsets from the matching build rather than guessing.

## First boot acceptance check

- HELLEBORE dashboard appears.
- Number keys open Wi-Fi, BLE, diagnostics, reports, simulation and terminal screens.
- Wi-Fi scan returns a result list in a location with nearby access points.
- BLE scan returns nearby advertisements.
- Reports screen can detect microSD and save a report.
- Terminal commands `help`, `about`, `status`, `clear`, `scanwifi`, `scanble`, and `report` behave as described.

Cloud CI proves that host tests and firmware compilation succeeded. Only the checks performed on the physical device prove hardware behavior.

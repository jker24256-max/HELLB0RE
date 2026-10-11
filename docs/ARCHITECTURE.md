# HELLEBORE architecture

## Principles

1. Hardware access stays behind service/module boundaries.
2. UI rendering is separate from input handling and command parsing.
3. Each module declares support state: planned, available, unsupported, or tested.
4. Radio features must not block the UI indefinitely.
5. Reports must disclose missing fields and failed writes instead of silently succeeding.
6. CI success means compilation and host tests passed; it does not certify physical-device behavior.

## Runtime layers

- **Platform adapter:** M5Cardputer initialization, display, keyboard, storage and radio APIs.
- **Core:** application state, routing, input mapping, capability flags and command parsing.
- **UI:** dashboard, module detail views, terminal and dialogs.
- **Services:** Wi-Fi, BLE, IR, diagnostics and report storage.
- **Modules:** user-facing workflows built on services.
- **Tests:** host-side pure-logic tests plus cloud firmware compilation and later hardware acceptance checks.

## v0.2 acceptance criteria

- CI host tests pass.
- PlatformIO compiles the Cardputer Adv target and merges a single full-flash image.
- Passive Wi-Fi audit listens on channels 1, 6, and 11 and reports frame counters.
- Boot reaches the dashboard.
- Keyboard input can select a module, open it and return.
- Terminal commands help/about/status/clear work.
- Unknown commands do not crash or hang.
- A firmware artifact is uploaded only after successful compilation.
- Hardware behavior remains marked unverified until tested on a physical device.

## Planned modules

- Wi-Fi: AP discovery, advertised security review, channel overview, authorized connectivity checks, suspicious duplicate-SSID observations.
- BLE: advertisement discovery, privacy observations, authorized GATT inspection on compatible peripherals.
- Diagnostics: device uptime, free heap, storage state and radio availability.
- Reports: versioned plain-text or CSV output to microSD, with explicit failure messages.
- Attack lab: controlled simulations and detection exercises; no jamming, disruptive deauthentication, credential theft, or password-capture portals.
- IR: supported send/receive workflows subject to board hardware and library support.

## UI model

The dashboard is the default screen. A selected module opens a detail page. The terminal is available from the dashboard and provides a small command set. Long-running operations will later use explicit states and timeouts rather than blocking loops.

# HELLEBORE feature status

This matrix distinguishes code that exists from hardware behavior that still needs a physical Cardputer Adv acceptance test.

## Implemented in firmware

- Dark dashboard, keyboard navigation, terminal, command parser and branded boot splash.
- Wi-Fi access-point discovery with SSID, BSSID, channel, RSSI and basic open/secured labeling.
- BLE advertisement discovery with names and RSSI.
- Passive Wi-Fi frame audit on channels 1, 6 and 11; counts management/data frame classes and records deauthentication/disassociation observations without transmitting disruptive frames.
- Connected-network diagnostics: local IP, gateway, DNS resolution, and TCP reachability checks to the local gateway on ports 80/443.
- microSD plain-text evidence reports with device health, scan totals, Wi-Fi inventory observations, channel counts, passive audit counters, network-check result, and companion stats.
- Helle offline companion: XP, levels, moods, small face, contextual hints, and a boot splash.
- GitHub Actions host tests, firmware compilation and merged full-flash image artifact.

## Planned / needs implementation

- Persistent companion profile across reboots, richer personality/event messages and configurable idle behavior.
- Theme settings, brightness controls, animation toggles and custom splash assets.
- JSON/CSV report export, report browser, report comparison and report integrity hashes.
- Wi-Fi channel utilization visualization, duplicate SSID correlation and more nuanced security advisories.
- BLE service UUID details and explicitly selected lab-peripheral GATT inspection.
- Target allowlist/profile management and small-scope authorized service checks.
- Offline knowledge base, guided assessment playbooks, and richer remediation guidance.
- Companion desktop report viewer, optional local sync, signed firmware-update workflow.
- IR workflows only after confirming the exact Cardputer Adv hardware/library path and adding safe controls.
- Hardware acceptance tests for display, keyboard, radio coexistence, microSD pinout, and flash/partition behavior.

## Companion design

Helle is an offline, lightweight field companion inspired by the *personality and progress loop* of Pwnagotchi, not a port of its Linux/AI stack. XP currently rewards local interactions and evidence workflow actions; it does not imply that a vulnerability was found. No cloud account or API key is required.

## Platform limits and safety

The ESP32-S3 is not a Linux computer. HELLEBORE does not claim full Kali/Wifite capability, built-in NFC/sub-GHz/NRF24 support, or guaranteed handshake capture. The firmware deliberately does not transmit jamming/deauthentication attacks or collect credentials. Detection and evidence collection are the default.

A green GitHub Actions build proves only that host tests and compilation passed. Verify actual hardware behavior before using the device for a client assessment or advertising a feature as supported.

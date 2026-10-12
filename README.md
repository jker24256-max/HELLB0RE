# HELLEBORE

A clean, minimal desk-companion firmware for the M5Stack Cardputer Adv. This is a fresh implementation, not the previous wireless-tool dashboard.

## Current design

- A custom HELLEBORE header and animated pixel mascot.
- Simple mood cycling and interaction counter.
- A 25-minute focus timer.
- Optional microSD persistence for the tiny companion state.
- A small utility screen, with a deliberate, calm cyan/gold visual identity.

## Keys

- `P` — interact with Helle.
- `F` — focus timer.
- `Space` — start/pause the focus timer.
- `R` — reset the focus timer.
- `M` or `T` — utility screen.
- `S` — save companion state to microSD.

## Build and flash

GitHub Actions builds the firmware for Cardputer Adv and merges the required binaries into `hellebore-full.bin`. Flash that merged image at `0x0` only after the latest workflow succeeds. This code has not been physically verified on hardware.

## Hardware note

The SD card is optional. Its CS pin configuration must be verified against the exact Cardputer Adv revision. The companion remains usable without a card.

## Scope

HELLEBORE is a desk companion first. This clean-slate revision intentionally omits the previous wireless reconnaissance menus and feature sprawl.

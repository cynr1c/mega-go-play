# Mega Go-Play
![Mega Go-Play](logo.png?raw=true "Mega Go-Play")

## About

Mega Go-Play is based on the improved Super Go-Play by mattkj. It features a re-written integer-based audio sample scaling that greatly reduces audible artifacts especially at low levels and is much lighter on the CPU. The 2 lowest audio levels are quieter than normal so a special quiet build should not be neccessary.
Below is the Super Go-Play readme:


Super Go-Play is a combination of two other Go-Play forks [(more info here)](https://forum.odroid.com/viewtopic.php?f=159&p=248618), plus the following changes:

- New in-game `VOLUME` button behavior (details below)
- New hour glass icon
- Gameboy palette cycling changes
- Reversed `MENU` button behavior
- New name, logo and firmware tile image

## Full Feature List

The full changes compared to Go-Play are:

- **NES** - improved perfomance
- **Gameboy** - Additional color palettes added. Switch between them with `SELECT + LEFT` and `SELECT + RIGHT`
- **SMS** - improved performance, `START` / `SELECT` buttons are swapped
- **MENU Button** - Hold `MENU` to exit game without saving
- **VOLUME Button** - In game, press `VOLUME` to mute/unmute audio. Press `VOLUME + UP` to increase volume. Press `VOLUME + DOWN` to decrease volume.
- New hour glass icon (50% larger, 100% more retro)
- New name, logo and firmware tile image
- Quiet version with reduced volume levels from "go-play-quiet"

## Download

- [Latest release is here](https://github.com/mattkj/super-go-play/releases/tag/v1.0.0)

## Installation

- Download the .fw file then follow [these instructions](https://wiki.odroid.com/odroid_go/write_app) from the Odroid wiki.

## Acknowledgements

- [@OtherCrashOverride](https://github.com/OtherCrashOverride/go-play) for "Go-Play" and everything else ODROID-GO related
- [@Cwiiis](https://github.com/Cwiiis/go-play/tree/wip/cwiiis/partial_updates) for NES and SMS performance improvements
- [@johannesbehr](https://github.com/johannesbehr/go-play) for START/SELECT swap on SMS and additional Gameboy color palettes
- [@gentleben](https://github.com/gentleben/go-play) for original MENU button change
- [@milesoberstadt](https://github.com/milesoberstadt/go-play-quiet) for "go-play-quiet"
- [@23pieces](https://github.com/23pieces/Odroid-Go-Tools) for "Odroid-Go-Tools"

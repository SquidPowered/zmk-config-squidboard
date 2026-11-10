# Squidboard ZMK configuration

This repository builds ZMK firmware for a wireless 28-key-per-side Squidboard
using nice!nano v2-compatible controllers. The left half is the split central.

## Provisional pinout

The PCB pinout was not available when this configuration was initialized. The
following assignments are deliberately grouped in the shield overlays so they
can be changed once the schematic is final.

| Function | nice!nano pins |
| --- | --- |
| Matrix rows | D4, D5, D6, D7, D8 |
| Matrix columns | D0, D1, D2, D3, D18, D21 |
| Left analog stick X/Y | D19/A1 (AIN0), D20/A2 (AIN5) |
| Right PMW3610 data / clock | D16/SDIO, D15/SCLK |
| Right PMW3610 CS / motion | D10/NCS, D9/MOT |
| Right PMW3610 power | 3V3/VDDIO, GND/GND |
| Right PMW3610 reset | 3V3/NRESET |

The matrix is electrically 5x6. Row 5 is sparse: column 0 is the palm key and
columns 3-5 are the thumb cluster on the left; the right side is mirrored by
the matrix transform. Diodes are configured `col2row`.

The analog stick is assumed to be a two-axis, 3.3 V PlayStation-style stick,
not a PS/2-protocol device. Its midpoint, range, deadzone, direction, and speed
are configured in `squidboard_left.overlay` and should be calibrated on the
assembled hardware.

## Default keymap

- QWERTY base layer with GUI/Alt/Ctrl/Shift home-row mods on `ASDF` and
  `JKL;`.
- Left thumb NAV layer with navigation, editing, media, and mouse buttons.
- Right thumb SYM layer with symbols, numbers, and function keys.
- Holding NAV and SYM together opens ADJ for Bluetooth profiles and reset.
- The joystick scrolls in every layer except GAME, where it sends WASD.
- The trackball always moves the mouse cursor.
- The outside palm keys are left and right Control by default.
- On ADJ, `H` latches the GAME layer and `J` returns to BASE. In GAME, the
  joystick sends WASD, the trackball remains the pointer, and right-hand keys
  provide left, right, middle, back, and forward mouse clicks.

Edit `boards/shields/squidboard/squidboard.keymap` to customize the layout.

## Build and flash

Push the repository to GitHub and download `squidboard_left.uf2` and
`squidboard_right.uf2` from the Actions build artifact. Put each controller in
its UF2 bootloader and copy the matching firmware to it. Flash both halves
after changing split or pointing-device configuration.

The PMW3610 driver is pinned to its ZMK 0.3-compatible branch. The analog input
module is pinned to a commit so upstream changes do not silently alter builds.

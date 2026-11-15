# OLED menu (128 × 32)

The interface uses four 6 × 8 character rows (21 characters per row). UP/DOWN
navigate, a short SELECT opens/confirms, and SELECT held for 700 ms goes back.
Button debounce is 30 ms. Direction repeat starts at 400 ms, repeats every
150 ms, and accelerates to 75 ms after two seconds. Both direction buttons
pressed together are ignored.

Home has position and velocity pages. SELECT opens Telemetry, Settings, System.
Telemetry adds a kicker voltage page. System contains the scrolling firmware
version and a reboot confirmation, initially selecting Cancel.

Settings blocks drive, dribbler, charging and kicking. Changing ID edits a draft
in the range 0–15. SELECT writes the ID immediately, preserving all other stored
settings and the existing signature, then applies it without reboot. Confirming
the unchanged ID does not write flash. Holding SELECT cancels the editor; holding
it again from Settings returns to the menu and releases the block. Pending radio
packets and old targets are discarded; new commands can then resume movement.
A reboot confirmation uses the same block. Navigation and telemetry do not
switch PLAY/TEST. In PLAY the OLED retains its last image. Hold any menu button
for over three seconds to enter TEST, then release the buttons to use the menu. The displayed velocities come from the existing robot state;
they are not newly added measured wheel feedback.

## Implementation

- `menu.hpp`: hardware-independent button timing and menu transitions.
- `screen.cpp`: device integration, bounded formatting, four-row rendering and
  changed-frame detection. Inputs run at 100 Hz; display submissions at most 10 Hz.
- OLED frame transfer uses I2C2 TX DMA1 Stream7 Channel7, with a dedicated copied
  frame buffer in the existing OLED library. The small address-window command is
  sent synchronously with a 2 ms timeout. DMA completion/error IRQs release the
  transfer, failures retry, and a 50 ms watchdog resets a stuck transfer.
- `Robot::set_control_locked()` clears commands and radio queues on both edges.
  Robot setters reject commands while blocked; `act()`, BLDC register output and
  kicker output also enforce the block. Physical power-off remains active.
- PLAY runs mainloop and settings broadcaster. The scheduled screen task returns
  after checking for a long press in PLAY. TEST runs the OLED interface, shell and monitor reset. `oledPrint` only accepts TEST and an idle OLED transfer;
  its temporary output remains until the next UI button event.

The EEPROM wrapper uses flash emulation and does not return a physical write
status. The on-screen Saved message indicates completion of that existing API;
power-cycle retention requires the board check below.

## Automated validation

Run from the repository root:

```sh
sh tests/host/run.sh
pio run -e disco_f429zi
```

Host tests cover debounce, hold/release separation, repeat timing and timer wrap,
menu boundaries, cancel/default reboot, ID wrap and duplicate-write suppression.
The actual screen integration is exercised with fake GPIO/OLED/storage: bounds
of every rendered string, large numbers and NaN, simultaneous directions,
scrolling version, preservation of stored settings, and shell takeover.
The actual Robot implementation is exercised with fake device endpoints for
lock/unlock, discarded targets, direct target mutation while blocked, and
resumption by a new command. These tests do not simulate STM32 DMA, CAN or flash.

## Board acceptance — NOT YET RUN

1. Check all pages on the 128 × 32 OLED: inverse selection, large ID, long version,
   negative/overflow values, short/long presses and simultaneous directions.
2. In PLAY, verify that the OLED retains its last image. In TEST with telemetry open, send motion commands. Enter Settings and check wheels and
   dribbler stop, charge and kick outputs deactivate, and incoming commands cannot
   restart them. Check the physical power-off button independently.
3. Cancel an edited ID; confirm the old ID remains. Save a different ID, leave
   Settings, and verify only new commands for the new ID work without reboot.
   Power-cycle and verify ID retention and unchanged other stored settings.
4. Leave Settings while commands are not being sent: no previous target may resume.
   Repeat for Cancel/back from reboot confirmation; then test confirmed reboot.
5. Observe `kernel::scheduler::get_tasks()` execution/period maxima for mainloop
   and screen, plus `devices::oled_drv.max_transfer_us` and `transfer_errors`, using
   the debugger. Compare telemetry with a static menu under the same radio load.
   Confirm no missed 10 ms control deadlines during normal display refresh.
   The DMA wire duration itself is expected to exceed 10 ms; it must not occupy
   that time in the CPU task. Record actual measurements before hardware acceptance.
6. Exercise OLED disconnect/reconnect and check retries, error counting and
   continued input/control processing. Check `oledPrint` rejection in PLAY and
   temporary output in TEST.

No device upload is part of the automated checks.

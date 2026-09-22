# Shared controller and simulator integration

`lib/robot_core` contains the controller, kinematics, odometry, NRFM command decoder, and kicker state machine. It has no STM32, Webots, shell, logging, or clock dependencies. PlatformIO and CMake compile the same sources.

`src/main.cpp` is the STM32 entry point. The existing device manager owns the board's drivers and initializes its adapters. `src/devices/robot/Robot` is a compatibility facade for shell/screen callers: it inherits the portable controller, collects hardware inputs, and applies outputs. Chassis and odometry wrappers also delegate to the portable library. Global devices remain board-local; controller instances do not share mutable state.

`sim/webots/main.cpp` is the host entry point, with Webots device access in `adapter.*`. Tests need neither an MCU SDK nor Webots.

## Builds

MCU:

```sh
pio run -e disco_f429zi
```

Host tests:

```sh
cmake -S . -B build/host -DBUILD_WEBOTS=OFF -DBUILD_TESTING=ON
cmake --build build/host --parallel
ctest --test-dir build/host --output-on-failure
```

Webots controller:

```sh
cmake -S . -B build/webots -DBUILD_WEBOTS=ON -DWEBOTS_HOME=/path/to/webots
cmake --build build/webots --parallel
```

`WEBOTS_HOME` must contain `include/controller/cpp` and `lib/controller`. It defaults to the environment variable. The executable is `build/webots/release/fb4_firmware`. Install/copy it into a Webots project's `controllers/fb4_firmware/fb4_firmware` directory and set the robot's `controller` to `fb4_firmware`. Alternatively, `make build` retains the repository's `fb4_controller` copy convention.

The old PlatformIO `webots_sim` environment is removed: it selected an MCU toolchain. Use CMake for host builds.

## Robot model configuration

Pass options through the robot's Webots `controllerArgs`. Example for wheel numbers 1 through 4:

```text
controllerArgs [
  "--wheels=1,2,3,4"
  "--signs=1,1,1,1"
  "--robot-id=15"
  "--velocity=0.3,0,0"
]
```

Wheel entries are always **front-left, rear-left, rear-right, front-right**. Each entry `N` resolves to `wheelN_motor` and `wheelN_encoder`, matching the supplied model naming convention. Defaults are `0,1,2,3`; set the order to match the model. `--signs` defines each model axis relative to the controller's canonical wheel direction and is applied to both encoder feedback and motor output. The existing MCU CAN adapter applies its own motor sign conversion.

Options:

| Option | Meaning |
| --- | --- |
| `--wheels=0,1,2,3` | Wheel number/suffix mapping in canonical order |
| `--signs=1,1,1,1` | Each wheel sign must be +1 or -1 |
| `--imu=device_name` | Optional Webots InertialUnit; absent by default |
| `--yaw-sign=1` | Convert IMU yaw to controller convention (+1 or -1) |
| `--receiver=device_name` | Optional Webots Receiver carrying one NRFM packet per message |
| `--dribbler=device_name` | Optional rotational motor; absent by default |
| `--robot-id=15` | NRFM robot ID, 0–15 |
| `--velocity=x,y,w` | Initial body velocity in m/s, m/s, rad/s |
| `--heading=delta` | Initial relative heading in radians; use after `--velocity` when combining |

Without an IMU, the shared estimator integrates heading from wheel velocities. No ground-truth pose is substituted. Adding an InertialUnit requires only setting its name and verifying the yaw sign. Geometry is supplied through `robot_core::Geometry` when constructing the controller; defaults preserve the firmware geometry and calibrated inverse matrix.

The dribbler is optional. Kicker physics and capacitor/ball sensors are not implemented in the supplied model. Their control decisions still execute in the core, but the Webots adapter reports unsupported requests instead of claiming to apply them. To add a kicker model, populate `SensorFrame::capacitor_voltage`, `ball_deep`, and `ball_front`, then consume `ActuatorFrame::kicker` in the adapter. These are persistent semantic levels; apply impulses on an appropriate transition, not on every tick with an active output. Map electrical pin polarity only in the MCU adapter.

## Control contract and timing

Each cycle calls `sense(frame)`, applies any accepted commands in arrival order, then calls `plan()` and `act()`. `act()` returns an `ActuatorFrame`. This preserves relative-heading commands being interpreted against the current sensed heading. Direct users can construct `RobotCommand` and call `apply_command`; raw protocol bytes are optional. Set `received_us` on commands and `timestamp_us` on sensor frames from the same clock.

The control period is 10 ms (`robot_core::control_period_us`). Webots `basicTimeStep` must divide 10 ms; incompatible worlds fail with a configuration error instead of running several control cycles over stale physics. Simulation time drives timers. The first encoder sample establishes a position baseline; measured velocities are available from the second sample. Encoders are differentiated using elapsed simulation time.

On MCU the cooperative scheduler invokes one cycle when due, without a busy-wait inside the control loop. Late cycles run once, with the original fixed 10 ms integration/smoothing interval; missed cycles are not replayed. Scheduler monitoring remains available to identify overruns. Timer subtraction remains wrap-safe for the existing 32-bit microsecond clock. The IMU driver currently does not expose read validity, so the MCU adapter retains its prior assumption that yaw is usable.

Sensor and actuator frames use radians, seconds, metres, m/s, rad/s, and volts. Body x/y follow the existing chassis equations. Old radio x/y map to controller y/-x as before. `wheels_measured` distinguishes actual encoders from the MCU's current command echo; it is metadata, not a control gate. `yaw_valid=false` selects wheel-based heading integration.

`reset()` clears controller state, timers, odometry, and kicker state while retaining configuration. Reconstruct the Webots adapter when restarting an episode or rewinding world time so encoder differentiation is also reset. No core method sleeps or reads wall time.

## Tests and limits

Host tests cover speed/acceleration limits, coordinate transforms, heading wrap, legacy kinematics and odometry values, packet formats/length checks, explicit command time, idle timer wrap, dribbler updates, kicker transitions, reset, and replay across independent instances. CI builds MCU firmware and runs host tests with address/undefined-behavior sanitizers; it does not publish releases.

The Webots build also produces an isolated adapter smoke test with a fixed base and four independent rotors. To run it:

```sh
cmake -S . -B build/webots -DBUILD_WEBOTS=ON -DRUN_WEBOTS_TESTS=ON -DWEBOTS_HOME=/path/to/webots
cmake --build build/webots --parallel
ctest --test-dir build/webots --output-on-failure
```

It exercises the actual SDK, controller, motors, and encoders for translation, rotation, combined, and stop commands. It verifies wheel tracking, not body trajectories or omni-wheel contact dynamics. The user robot world is still required for trajectory/heading validation and actuator-model calibration. Hardware execution time and physical actuator behavior require board testing.

Behavior changes made with the extraction:

- STM32 now enters the scheduler after initialization; the previous entry point returned without running it.
- Controller state is explicitly initialized even when EEPROM defaults are used.
- Packet lengths are checked before reads; oversized radio payloads are rejected before copying.
- Raw debug motor writes remain MCU-only. Their payload is now bounded by CAN's eight-byte limit and uses `length - 5` for the existing five-byte header, avoiding the old one-byte over-read.
- Shell packet generation uses explicit bytes, preserving its historical shell-to-radio flag mapping.

Intentionally retained: the hard-coded inverse kinematics matrix, the existing body/global odometry integration behavior, idle semantics (idle changes motor settings rather than stopping the robot), the incomplete coordinate mode, and the EEPROM settings layout/signature scheme. Those require separate behavior changes with their own validation.

## Validation in this workspace

- Baseline MCU build: 68,960 bytes flash, 58,112 bytes RAM.
- Refactored MCU build: 71,440 bytes flash, 58,400 bytes RAM.
- Host core tests: passed, including AddressSanitizer and UndefinedBehaviorSanitizer.
- Webots R2025a: controller and smoke-test builds passed; all five rotor scenarios passed using the actual SDK with `QT_QPA_PLATFORM=offscreen`.
- No MCU flashing, on-board timing measurements, or user-world trajectory tests were performed.

The smoke controller writes `smoke-progress.log` in its generated controller directory for diagnosis. On a headless machine, an appropriate Qt platform/display setup may be needed to launch Webots even with rendering disabled.

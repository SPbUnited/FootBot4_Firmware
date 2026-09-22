# Shared firmware and simulator control: refactoring plan

Extract one portable C++17 control library, compiled from the same sources by PlatformIO and CMake. Give STM32 and Webots separate entry points and adapters. Start with wheel feedback and wheel velocity commands as the simulation boundary so the simulator exercises the firmware's kinematics, odometry, and motion controller.

Implementation status: the shared core, MCU adapters, separate Webots entry point, portable packet decoder, host regression tests, and build/test CI are implemented. See [integration documentation](simulator-integration.md) for the resulting contract and remaining model-level validation. The following records the original source-based plan. Webots is assumed to be the initial simulator because the repository already references it. No Webots world or robot device mapping was found in the inspected source tree; those are integration inputs to obtain before implementing its adapter.

## Current obstacles

| Location | Finding | Planned change |
| --- | --- | --- |
| `src/main.cpp` | Webots only advances time; its loop does not call control logic. MCU scheduler initialization is commented out. | Separate entry points, each explicitly invoking the shared controller. Establish intended MCU startup behavior before changing it. |
| `CMakeLists.txt` | Shallow source glob omits nested control implementations; global flags and include paths select Webots. | Explicit portable-library and adapter targets, with target-local dependencies. |
| `platformio.ini` | `webots_sim` still selects an STM32 board/framework. | Keep PlatformIO for MCU and document CMake as the host build. Remove the misleading environment once the replacement is working. |
| `devices/device_manager.*` | Global object graph; Webots replacement only constructs `webots::Robot`. | Platform-specific composition roots that own adapters and controller instances. |
| `devices/robot` | Algorithm state and concrete BLDC, IMU, chassis, and kicker references are mixed. `act()` includes limits, smoothing, hardware time, and shutdown GPIO. | Move state and algorithms into the core; read inputs and apply outputs outside it. |
| `devices/chassis`, `devices/odom` | Kinematics depends on BLDC types; chassis implementation is under `BARE_METAL`. Odometry inherits that dependency through headers. | Portable geometry, wheel types, transforms, and estimator. |
| `devices/nrfm_decoder` | Parsing mutates `Robot`, toggles GPIO, and supports raw motor register writes. | Pure packet decoding to typed commands; LED and raw debug operations handled by platform services. |
| `devices/kicker` | State machine reads global robot commands, clock, ADC, and pins. | Shared decision/state logic with explicit feedback and time; electrical operations in STM32 adapter. |
| `kernel`, `apps/mainloop` | Scheduler, logging, shell, and HAL dependencies spread into control code; main loop busy-waits. | Scheduling and diagnostics outside the portable control step. |

## Target boundary

```text
STM32 main + scheduler                 Webots main + simulation stepping
          |                                        |
STM32 sensors / command transport      Webots sensors / command transport
          |                                        |
          +---- SensorFrame + typed commands ------+
                               |
                     shared RobotController
                  estimator -> control -> kinematics
                               |
                        ActuatorFrame
                               |
          +--------------------+-------------------+
          |                                        |
STM32 CAN / kicker / power              Webots motors / kicker model
```

The core must include neither HAL nor Webots headers, device managers, shell exports, or platform-selection macros. Use plain data at its boundary; interfaces are only needed for actual runtime services. Avoid creating simulator implementations of SPI, CAN, GPIO, or UART merely to make algorithms compile.

Suggested layout:

```text
lib/robot_core/
  library.json
  CMakeLists.txt
  src/robot_core/       # public types, controller, kinematics, odometry, protocol
  src/                 # implementations
src/platform/stm32/    # entry point, composition, adapters and board services
sim/webots/            # entry point, composition, device mapping and adapters
test/core/            # deterministic tests and recorded-input replay
test/integration/     # Webots scenarios
```

Existing drivers can stay where they are during extraction. Move them after dependencies are clean; do not combine directory changes with algorithm changes.

Define these contracts before implementing adapters:

- `SensorFrame`: four measured wheel angular velocities, IMU yaw, ball sensors, capacitor voltage, shutdown input, and timestamps/validity for asynchronous feedback.
- `RobotCommand`: typed control modes and targets, dribbler/kicker requests, plus explicit command receipt time. Preserve partial-command semantics rather than resetting unrelated targets on every packet.
- `ActuatorFrame`: four wheel targets, dribbler target, semantic kicker outputs, idle state, and power-off request. Specify which outputs are persistent levels and which are events to prevent repeated kicks.
- `RobotState` and `RobotSettings`: instance-owned state and portable configuration; explicit reset initializes every member.
- A control tick takes inputs and explicit time, and returns outputs. Preserve the current sense → command application → plan → act ordering initially, including relative-angle command interpretation.

Document units, wheel order (`fl`, `rl`, `rr`, `fr`), coordinate axes, yaw sign, and angle wrapping. Put model-specific wheel signs and sensor conversions in adapters. Preserve the existing negative BNO055 yaw convention until coordinate tests establish the intended mapping.

Keep the initial controller period at 10 ms, currently defined by `Ts_us`. Move this into portable configuration. The core must not sleep or read a clock. The MCU scheduler supplies time; Webots supplies simulation time, so pausing simulation also pauses controller timers. Use a Webots physics step compatible with 10 ms control ticks where possible. If that is impossible, specify and test the chosen timing policy; repeated controller ticks over unchanged sensor data do not recreate finer physics. Define late-tick behavior and preserve wrap-safe elapsed-time handling on MCU.

## Incremental implementation sequence

1. **Establish baseline and contracts.** Record MCU and CMake build results, current initialization, packet fixtures, geometry, units, and representative input/output traces. Confirm why scheduler startup is disabled. Identify the Webots world, wheel devices, sensors, and command source. Separate existing defects from extraction changes.
   - Exit: reproducible baseline and documented control/simulator contracts, including known failures.

2. **Extract portable math and state.** Move wheel/body types out of BLDC headers. Split chassis into pure forward/inverse kinematics and motor I/O. Extract odometry and configuration without hardware includes. Initialize all state explicitly; current global zero-initialization must not become an accidental requirement for stack-allocated controllers.
   - Exit: CMake builds and tests this library without STM32 or Webots installed; MCU uses the same implementations.

3. **Extract the motion controller.** Replace concrete device references in `Robot` with input frames and output frames. Move limits, acceleration smoothing, angle control, idle timing, and dribbler decisions into the core. Move sensor reads, GPIO shutdown, and CAN writes into a thin MCU runner. Have the existing MCU scheduler invoke a nonblocking tick.
   - Exit: recorded-input replay matches the original motion calculations within chosen float tolerances; MCU links and retains its configuration and peripheral initialization.

4. **Separate commands and auxiliary control.** Decode NRFM bytes into commands independently of radio transport and LED feedback. Validate lengths before reading bytes; replace compiler-dependent packet bitfields with explicit decoding backed by packet fixtures. Route raw register overrides to an MCU debug service. Extract kicker state transitions with explicit command, feedback, and time. Keep ADC calibration, pin polarity, and electrical timing in its adapter. Keep flash serialization and shell registration outside the core; expose state snapshots/settings operations for screen and shell clients. Preserve periodic motor settings updates currently performed by `setting_broadcaster`.
   - Exit: host tests cover command decoding and kicker transitions; MCU command and diagnostic paths remain connected.

5. **Integrate Webots.** Construct a controller instance and Webots adapters in `sim/webots`. Read simulated wheel/IMU feedback, apply the same typed commands, run the shared tick, and write wheel outputs. Use ground truth for scoring rather than silently replacing the estimator. Model dribbler/kicker behavior at the declared fidelity; mark unsupported features explicitly. Reset controller and adapter state together between episodes.
   - Exit: translation, rotation, combined motion, and heading commands run through the actual shared controller in simulation with repeatable results.

6. **Finish build separation and regression checks.** Make CMake build `robot_core`, SDK-independent tests, and an optional Webots executable. Use explicit source lists and target-local includes/definitions; discover and validate Webots only when requested. Make PlatformIO compile the local core library and STM32 sources, excluding host entry points. Retain the custom linker script and letter-shell setup. Remove obsolete `.mock.cpp` implementations and platform conditionals from extracted code. Add build/test CI independently of the existing workflow's release/tag publishing steps.
   - Exit: clean MCU build, clean host tests without SDKs, and clean Webots build with documented SDK configuration.

Deliver each stage as a reviewable change. Introduce only the build wiring needed for each extraction, then finish cleanup in stage 6; both build paths should stay usable throughout migration.

## Validation and behavior changes to track separately

- Characterize the hard-coded `Jinv` against configured geometry before replacing it. A forward/inverse round trip may expose an existing mismatch; extraction should not silently recalibrate the robot.
- Confirm local/global frame handling in odometry: current `sense()` passes chassis velocity directly to integration, then overwrites heading from IMU. Correcting this may change trajectories and belongs in an explicit algorithm change.
- Test local/global velocity modes, heading wrap near ±π, speed limits, acceleration limits, command timing, and idle transitions. `COORDINATE` is currently a TODO; implementing it is outside the separation work.
- Test malformed/truncated packets and the existing wire formats, including half-float data. The current decoder accesses bytes before validating packet length.
- Test deterministic reset/replay and two controller instances with no shared mutable state.
- Preserve EEPROM compatibility during extraction. The current signature hashes object representation; introduce versioned serialization only with an explicit migration policy.
- Check MCU RAM/flash and control tick duration after extraction. A host build does not establish embedded timing correctness.
- Simulator tests cover controller behavior at the modeled actuator boundary. Motor-controller inner loops, bus/interrupt timing, and physical kicker behavior still require device-level validation.

Proposed final commands (to be implemented, not currently available):

```sh
pio run -e disco_f429zi
cmake -S . -B build/host -DBUILD_TESTING=ON -DBUILD_WEBOTS=OFF
cmake --build build/host
ctest --test-dir build/host --output-on-failure
cmake -S . -B build/webots -DBUILD_WEBOTS=ON -DWEBOTS_HOME=/path/to/webots
cmake --build build/webots
```

Completion means the same controller sources execute on MCU, in SDK-independent host tests, and in Webots; only input acquisition, output application, scheduling, and platform services differ.

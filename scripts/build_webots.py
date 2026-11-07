"""Expose the host CMake build in PlatformIO's Custom project tasks."""

import os
from pathlib import Path
import shutil
import subprocess

Import("env")


def build_webots(source, target, env):
    project = Path(env.subst("$PROJECT_DIR"))
    build = project / "build" / "platformio-webots"
    cmake = os.environ.get("CMAKE_COMMAND") or shutil.which("cmake")
    if not cmake:
        print("Webots build requires CMake on PATH (or set CMAKE_COMMAND).")
        return 1

    # Use the host process environment, not the STM32 SCons compiler settings.
    configure = [
        cmake, "-S", str(project), "-B", str(build),
        "-DBUILD_WEBOTS=ON", "-DBUILD_TESTING=OFF", "-DRUN_WEBOTS_TESTS=OFF",
        "-DCMAKE_BUILD_TYPE=Release",
    ]
    if os.environ.get("WEBOTS_HOME"):
        configure.append("-DWEBOTS_HOME=" + os.environ["WEBOTS_HOME"])

    try:
        subprocess.run(configure, cwd=project, check=True)
        subprocess.run(
            [cmake, "--build", str(build), "--config", "Release",
             "--target", "fb4_firmware", "--parallel"],
            cwd=project, check=True,
        )
        filename = "fb4_firmware.exe" if os.name == "nt" else "fb4_firmware"
        executable = build / "release" / filename
        if not executable.is_file():
            # Multi-configuration generators append the configuration directory.
            executable = build / "release" / "Release" / filename
        destination = project / ("fb4_controller.exe" if os.name == "nt" else "fb4_controller")
        shutil.copy2(executable, destination)
        print("Webots controller: " + str(destination))
    except subprocess.CalledProcessError as error:
        return error.returncode
    except OSError as error:
        print("Webots build failed: " + str(error))
        return 1
    return 0


env.AddCustomTarget(
    name="webots",
    dependencies=None,
    actions=build_webots,
    title="Build Webots controller",
    description="Configure and build the host controller with CMake",
    always_build=True,
)

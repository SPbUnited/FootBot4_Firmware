.PHONY: build host test clean

# CMake owns host builds; PlatformIO owns the MCU build.
build:
	cmake -S . -B build/webots -DBUILD_WEBOTS=ON
	cmake --build build/webots
	cp build/webots/release/fb4_firmware fb4_controller

host:
	cmake -S . -B build/host -DBUILD_WEBOTS=OFF -DBUILD_TESTING=ON
	cmake --build build/host

test: host
	ctest --test-dir build/host --output-on-failure

clean:
	cmake -E rm -rf build
	cmake -E rm -f fb4_controller

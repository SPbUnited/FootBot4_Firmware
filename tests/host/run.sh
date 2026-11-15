#!/bin/sh
set -eu
cd "$(dirname "$0")/../.."
build_dir=$(mktemp -d /tmp/footbot-host-tests.XXXXXX)
trap 'rm -rf "$build_dir"' EXIT
c++ -std=c++17 -Wall -Wextra -Werror -Isrc tests/host/menu_test.cpp -o "$build_dir/menu_test"
"$build_dir/menu_test"
c++ -std=c++17 -Itests/host/fakes -Isrc tests/host/robot_lock_test.cpp src/devices/robot/robot.cpp -o "$build_dir/robot_lock_test"
"$build_dir/robot_lock_test"
c++ -std=c++17 -Itests/host/ui_fakes -Isrc '-DFIRMWARE_VERSION="very-long-firmware-version-for-scroll-test"' tests/host/screen_test.cpp src/apps/screen/screen.cpp -o "$build_dir/screen_test"
"$build_dir/screen_test"

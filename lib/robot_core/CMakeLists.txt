add_library(robot_core STATIC
    src/kinematics.cpp
    src/odometry.cpp
    src/robot.cpp
    src/kicker.cpp
    src/protocol.cpp
)
target_include_directories(robot_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_compile_features(robot_core PUBLIC cxx_std_17)

import serial
import struct
import time

PORT = "/dev/ttyACM0"
BAUD = 115200

ser = serial.Serial(PORT, BAUD, timeout=0.1)

# RX packet: <5eB? = 5 half-floats + 1 byte + 1 bool = 12 bytes
# TX telemetry: <Be?? = 5 bytes

def send_command(vel_x, vel_y, w, dribbler, kicker, kicker_mode, angle_mode):
    """Send command packet in new binary format."""
    packet = struct.pack('<5eB?',
                         vel_x, vel_y, w,
                         dribbler, kicker,
                         kicker_mode, angle_mode)
    ser.write(packet)

def read_telemetry():
    """Read and unpack a telemetry packet."""
    data = ser.read(5)
    if len(data) != 5:
        return None
    robot_id, voltage, ball_deep, ball_front = struct.unpack('<Be??', data)
    return {
        'robot_id': robot_id,
        'voltage': voltage,
        'ball_deep': ball_deep,
        'ball_front': ball_front,
    }

print(f"Sending commands on {PORT} @ {BAUD}")
last_sent = time.time()
try:
    while True:
        send_command(0.0, 0.0, 0.0, 0.0, 0.0, 0, False)
        print("sent TX! in", time.time() - last_sent)
        last_sent = time.time()

        tel = read_telemetry()
        if tel:
            print(f"RX: {tel}")

        time.sleep(0.01)
except KeyboardInterrupt:
    print("\nStopped.")
finally:
    ser.close()

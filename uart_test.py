import serial
import json
import time

PORT = "/dev/ttyACM0"
BAUD = 115200

ser = serial.Serial(PORT, BAUD, timeout=0.1)

cmd = {
    "xvel": 0.0,
    "yvel": 0.0,
    "wvel": 10,
    "dribbler": 0,
    "voltage": 0,
    "kick_lower": False,
    "kick_upper": False,
    "autokick_lower": False,
    "autokick_upper": False,
    "autokick_momentum": False,
    "angle_mode": False,
}

print(f"Sending commands on {PORT} @ {BAUD}")
last_sent = time.time()
try:
    while True:
        # Send command
        payload = json.dumps(cmd, separators=(",", ":")) + "\n"
        ser.write(payload.encode())
        print("sent TX! in",(time.time()-last_sent))
        last_sent = time.time()
        # Read telemetry
        if ser.in_waiting:
            line = ser.readline().decode("utf-8", errors="replace").strip()
            if line:
                print(f"RX: {line}")

        time.sleep(0.01)
except KeyboardInterrupt:
    print("\nStopped.")
finally:
    ser.close()

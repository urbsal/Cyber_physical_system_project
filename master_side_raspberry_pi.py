import serial
import time
import sys
import json
from datetime import datetime
import glob



def find_arduino_port():
   #linux system used dev to search for port 
    patterns = ['/dev/ttyACM*', '/dev/ttyUSB*', '/dev/ttyAMA*']
    for pattern in patterns:
        ports = glob.glob(pattern)
        if ports:
            return sorted(ports)[0]
    return None

def connect_to_arduino():
    #established serial communication
    port = find_arduino_port()
    if not port:
        print("Arduino not found! Please connect via USB.")
        return None

    try:
        ser = serial.Serial(
            port=port,
            baudrate=9600,
            timeout=1
        )
        time.sleep(2)  # Wait for Arduino reset
        ser.reset_input_buffer()
        ser.reset_output_buffer()
        print(f"Connected to Arduino at {port}")
        return ser
    except Exception as e:
        print(f"Connection error: {e}")
        return None

def send_command(ser, command):
    """Send single character command to Arduino"""
    try:
        ser.write(command.encode('ascii'))
        ser.flush()
        print(f"[{datetime.now().strftime('%H:%M:%S')}] Sent command: '{command}'")
        return True
    except Exception as e:
        print(f"Send error: {e}")
        return False



def main():
    print("=== Raspberry Pi Master ===")
    ser = connect_to_arduino()
    if not ser:
        sys.exit(1)

    buffer = ""
    fan_state = False
    last_fan_command = False

    try:
        while True:
            if ser.in_waiting:
                char = ser.read(1).decode('ascii', errors='ignore')
                buffer += char

                if char == '\n':
                    line = buffer.strip()
                    buffer = ""

                    if line:
                        print(f"[{datetime.now().strftime('%H:%M:%S')}] Arduino: {line}")

                        try:
                            data = json.loads(line) #the json file converted in dictionary 
                            temp = data.get("temp", 0)
                            pir = bool(data.get("pir", 0))
                            fan = bool(data.get("fan", 0))
                            ts = data.get("ts", 0)

                            print(f" Sensor Data: Temp={temp}°C, PIR={'ON' if pir else 'OFF'}, Fan={'ON' if fan else 'OFF'}")

                           #Turn on the fan if pir and temp is above >20 
                            if pir and temp > 20:
                                fan_state = True
                            else:
                                fan_state = False

                            # Send command if state changed
                            if fan_state != last_fan_command:
                                send_command(ser, 'F' if fan_state else 'O')
                                last_fan_command = fan_state
                            else:
                                print(f" No change (Fan already {'ON' if fan_state else 'OFF'})")

                        except json.JSONDecodeError:
                            print(" Failed to parse JSON from Arduino")

            time.sleep(0.01)  
    except KeyboardInterrupt:
        print("\nShutting down...")
        try:
            send_command(ser, 'O')
            time.sleep(0.5)
        except:
            pass
        if ser.is_open:
            ser.close()
        print("Master terminated")

if __name__ == "__main__":
    main()

import serial
import subprocess
import time

LOCK_AC = "Lock_active.vbs"
LOCK_DC ="Lock_disconnect.vbs"
LOCK_OV_AC ="Lock_Startup.vbs"
LOCK_OV_DC ="Lock_Terminated.vbs"

arduino = None

def connect_serial():
    global arduino
    try:
        arduino = serial.Serial('COM9', 9600, timeout=1)
        subprocess.run(["cscript",LOCK_OV_AC],shell = True)
        print("Serial port connected successfully.")
    except :
        print("Failed to connect to serial port.")
        subprocess.run(["cscript",LOCK_OV_DC],shell = True)
        arduino = None

while True:
    if arduino is None or not arduino.is_open:
        print("Serial port is not connected. Retrying in 1 minute...")
        time.sleep(5)
        connect_serial()
        continue
    try:
        data = arduino.readline().decode().strip()
        if data == "active":
            subprocess.run(["cscript",LOCK_AC],shell = True)
        elif (data == "deactivate"):
            subprocess.run(["cscript",LOCK_DC],shell = True)
    except :
        print("Lost connection to serial port.")
        subprocess.run(["cscript",LOCK_OV_DC],shell = True)
        arduino = None
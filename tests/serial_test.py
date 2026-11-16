import serial, sys

port = sys.argv[1] if len(sys.argv) > 1 else '/dev/ttyUSB0'   # Windows: COM3
ser = serial.Serial(port, 115200, timeout=1)

while True:
    line = ser.readline().decode(errors='ignore').strip()
    if line:
        t, n, ax, ay, az = map(int, line.split(','))
        print(f"t={t} ms  n={n}  ax={ax} ay={ay} az={az}")

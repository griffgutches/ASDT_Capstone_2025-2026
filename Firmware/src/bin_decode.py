import struct
import csv


# < = Little Endian (Standard for RP2040/ESP32)
# I = uint32_t (Timestamp)
# f = float (Temp)
# f = float (Pressure)
# h = int16_t (Acc X)
# h = int16_t (Acc Y)
# h = int16_t (Acc Z)
# h = int16_t (Gyro X)
# h = int16_t (Gyro Y)
# h = int16_t (Gyro Z)
# h = int16_t (Mag X)
# h = int16_t (Mag Y)
# h = int16_t (Mag Z)
# Change for esp32 as the data changes
struct_format = '<Iffhhhhhhhhhh' 
struct_size = struct.calcsize(struct_format)

input_file = 'data114.bin'
output_file = 'readings114.csv'

with open(input_file, 'rb') as f, open(output_file, 'w', newline='') as out:
    writer = csv.writer(out)
    # Write Header
    writer.writerow(['ms', 'temp', 'press', 'accX', 'accY', 'accZ', 'gyroX', 'gyroY', 'gyroZ', 'magX', 'magY', 'magZ', 'test'])
    
    while True:
        data = f.read(struct_size)
        if len(data) < struct_size:
            break # End of file
        
        # Unpack the binary chunk into a Python tuple
        unpacked = struct.unpack(struct_format, data)
        writer.writerow(unpacked)

print(f"Conversion complete! Check {output_file}")
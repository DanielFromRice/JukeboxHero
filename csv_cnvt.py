import numpy as np
import sys


if len(sys.argv) != 4:
    raise Exception("Please run with format: python3 csv_cnvt.py [beambreak].csv [notes].csv [out].bin")

bytes_ip = bytearray([])

# BEGIN READING BEAM PRELOAD ARRAY

input_beams = np.genfromtxt(sys.argv[1], delimiter= ',').astype(np.int)

if input_beams.shape[1] != 4:
    raise Exception("Incorrect shape")

size = input_beams.shape[0]
print(f"size: {size}")
bytes_ip.append(size)

for row in range(input_beams.shape[0]):
    val = (8 * input_beams[row, 0]) + (4 * input_beams[row,1]) + (2 * input_beams[row,2]) + input_beams[row,3]
    print(f"beam val: {val}")
    bytes_ip.append(val)

# BEGIN READING IN NOTE PACKETS

input = np.genfromtxt(sys.argv[2], delimiter = ',').astype(np.int16)

if input.shape[1] != 6:
    raise Exception("Incorrect shape")

size = input.shape[0]
print(f"size: {size}")
bytes_ip.append(size)

for row in range(input.shape[0]):
    val = int(input[row,0]).to_bytes(1, 'little')
    print(f"val: {input[row,0]}, hex: {val}")
    bytes_ip.extend(val)
    for col in range(1, 5):
        val = int(input[row,col]).to_bytes(2, 'little')
        print(f"val: {input[row,col]}, hex: {val}")
        bytes_ip.extend(val)
    val = int(input[row,5]).to_bytes(1, 'little')
    print(f"val: {input[row,5]}, hex: {val}")
    bytes_ip.extend(val)


print(bytes_ip)

with open(sys.argv[3], "wb") as fil: # if we want as mixed size
    fil.write(bytes_ip)

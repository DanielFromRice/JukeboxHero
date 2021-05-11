import numpy as np

input = np.genfromtxt("sampledata.csv", delimiter = ',').astype(np.int16)

print(input)
print(input.tobytes()) # option 1

with open("out.bin", 'wb') as fil: # if we want as all 16bit
    fil.write(input.tobytes())

bytes_ip = bytearray([])

for num in input:
    val = int(num).to_bytes((int(num).bit_length() + 7) // 8, byteorder='little')
    print(val)
    bytes_ip.extend(val)

print(bytes_ip)

with open("out2.bin", "wb") as fil: # if we want as mixed size
    fil.write(bytes_ip)

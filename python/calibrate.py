import can
import numpy as np

can0 = can.interface.Bus(
    bustype="pcan", channel="PCAN_USBBUS1", bitrate=1000000)

avg = [0.0 for i in range(16)]
total_count = 500.0
count = int(total_count)
while count > 0:
    msg = can0.recv()
    if (msg is not None):
        sid = msg.arbitration_id
        if sid in [0x405, 0x407, 0x409, 0x40b]:
            if sid == 0x405:
                count -= 1
            data = [(float(int.from_bytes(msg.data[i:i+2],
                                          byteorder="little")) / total_count) for i in range(0, 8, 2)]
            for i, d in enumerate(data):
                avg[2*(sid-0x405)+i] += d

ch_order = [11, 15, 14, 12, 9, 13, 8, 10, 6, 7, 4, 5, 2, 0, 3, 1]
avg_vals = [int(avg[o]) for o in ch_order]

with open("../calibration.txt", "w+") as f:
    for val in avg_vals:
        f.write("{}\n".format(val))
print("Calibration file saved to ../calibration.txt!\n")
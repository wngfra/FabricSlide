import can
import numpy as np
import signal


rev = True


def exit_prog():
    global rev
    rev = False


bus = can.interface.Bus(
    bustype="pcan", channel="PCAN_PCIBUS2", bitrate=1000000)
filters = [{'can_id': 0x405, 'can_mask': 0xff0, 'extended': False}]
bus.set_filters(filters)

channel_order = [11, 15, 14, 12, 9, 13, 8, 10, 6, 7, 4, 5, 2, 0, 3, 1]

signal.signal(signal.SIGINT, exit_prog)

while rev:
    data = np.zeros(16, dtype=np.int)
    for i in range(4):
        msg = bus.recv()
        tid = int((msg.arbitration_id - 0x405) * 2)
        data[tid:tid+4] = [int.from_bytes(msg.data[i:i+2], byteorder="little") for i in range(0, 8, 2)]
    data = data[channel_order]
    print('Tactile readings: ', data)

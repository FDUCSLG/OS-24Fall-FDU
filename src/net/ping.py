"""
A server that receives a message from E1000,
then sends \"this is a ping!\" to it.

To use it, just \"make ping\" in the build directory.
"""

import socket
import sys
import time

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
addr = ('localhost', int(sys.argv[1]))
buf = "this is a ping!\n".encode('utf-8')

# receive a message, then ping.
sock.bind(addr)
data, raddr = sock.recvfrom(4096)
print(f'received {data.decode('utf-8')}.\n')

while True:
        print("pinging...", file=sys.stderr)
        sock.sendto(buf, raddr)
        time.sleep(1)

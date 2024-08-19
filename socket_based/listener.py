#!/usr/bin/env python3

import socket
import struct

MCAST_GRP = 'ff08::1'
MCAST_PORT = 5007
IS_ALL_GROUPS = True

sock = socket.socket(socket.AF_INET6, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEPORT, 1)
ifidx = struct.pack('I', socket.if_nametoindex('wlo1'))
sock.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_MULTICAST_IF, ifidx)

# sock.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_MULTICAST_LOOP, True)

mreq = struct.pack('16s15s'.encode('utf-8'),
                   socket.inet_pton(socket.AF_INET6, MCAST_GRP),
                   (chr(0) * 16).encode('utf-8'))
sock.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_MULTICAST_HOPS, 20)
sock.setsockopt(socket.IPPROTO_IPV6, socket.IPV6_JOIN_GROUP, mreq)

sock.bind(('::', MCAST_PORT))

while True:
  # For Python 3, change next line to "print(sock.recv(10240))"
  print(sock.recv(20))

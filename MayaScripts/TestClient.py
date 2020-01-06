import socket
import sys
import select
import socket
import sys


# Create a TCP/IP socket
server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Connect the socket to the port where the server is listening
server_address = ('localhost', 65432)
print (sys.stderr, 'connecting to %s port %s' % server_address)
server.connect(server_address)
server.setblocking(False)

inputs = [server]
outputs = []

def Tick():
    readable, writable, exceptional = select.select(inputs, [], [])
    while len(readable) > 0:
        for s in readable:
            data = s.recv(4096)
            if data:
                return data
            readable, writable, exceptional = select.select(inputs, [], [],0)
    
print Tick()
from socket import *
import time

address = ('10, 0, 0, 69', 1682)
client_socket = socket(AF_INET,SOCK_DGRAM)
client_socket.settimeout(1)

while(1):

    data = "caocao"
    client_socket.sendto(data, address)

    try:
        rec_data, addr = client_socket.recvfrom(32)
        byteData = bytes(rec_data)
        processedData = byteData[]
    except:
        pass
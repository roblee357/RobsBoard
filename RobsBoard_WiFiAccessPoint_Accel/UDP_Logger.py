import socket, time

UDP_IP = "192.168.1.115"
UDP_PORT = 80

sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
sock.bind((UDP_IP, UDP_PORT))
previous_second = 0
i = 0
rate = 0
while True:
    second = round(time.time())
    if second > previous_second:
        rate = i
        i = 0
    data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
    print(data, rate)
    previous_second = second 
    i += 1

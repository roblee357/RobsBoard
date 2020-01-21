"""
Brightness 
by Rusty Robison. 

Brightness is the relative lightness or darkness of a color.
Move the cursor vertically over each bar to alter its brightness. 
"""

barWidth = 20
lastBar = -1
previous_second = 0
i = 0
rate = 0
roll = 0
pitch = 0
yaw = 0
    
import socket, time

def setup():
    global sock
    size (1000, 700, P3D)
    colorMode(HSB, 100)
    #colorMode(HSB, width, 100, width)
    noStroke()
    background(0)
    UDP_IP = "192.168.1.115"
    UDP_PORT = 80
    frameRate(200)
    
    sock = socket.socket(socket.AF_INET, # Internet
                        socket.SOCK_DGRAM) # UDP
    sock.bind((UDP_IP, UDP_PORT))
    sock.setblocking(0)



def draw():
    global lastBar, previous_second, i, rate, pitch, roll, yaw, data, addr

    translate(width/2, height/2, 0)
    background(233)
    textSize(22)
    text("Roll: " + str(roll) + "     Pitch: " + str(pitch), -100, 265)
    
     
    
    # Rotate the object
    rotateX(radians(-pitch))
    rotateZ(radians(roll))
    rotateY(radians(yaw))
    
    # 3D 0bject
    textSize(30) 
    fill(0,76,153)
    box (386, 40, 200) # Draw box
    textSize(25)
    fill(255,255,255)
    text("Rob's Board", -183, 10, 101)
    second = round(time.time())
    if second > previous_second:
        rate = i
        i = 0
    #sock.select()
    try:
        # data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
        data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
        roll = float(data.split(',')[0])
        pitch = float(data.split(',')[1])
        yaw = float(data.split(',')[2])
        print(roll,data, rate,addr)
        previous_second = second 
        i += 1
    except:
        print('missed packet')

from mpl_toolkits.mplot3d import axes3d
import requests, time
from bs4 import BeautifulSoup 
from IPython.display import clear_output
import numpy as np
import matplotlib.pyplot as plt
  
# Create plot
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
ax = fig.gca(projection='3d')

def read_stream(): 
    # the target we want to open     
    #url='http://www.hindustantimes.com/top-news'
    url = 'http://192.168.4.1'
    #open with GET method 
    resp=requests.get(url) 
    clear_output()
    return resp.text
    #http_respone 200 means OK status 

i = 0
timeout = time.time() + 10 
while True:          
    stream = read_stream()
    # Roll, Pitch, Heading, AccX, AccY, AccZ 
    orient = [ float(x) for x in stream.split(',')[:3]]
    accel  = [ float(x) for x in stream.split(',')[3:]]
    x, y, z = orient
    ax.scatter(x, y, z, alpha=0.8, c="green", edgecolors='none', s=30, label="orient")
    plt.title('Matplot 3d scatter plot')
    plt.legend(loc=2)
    plt.show()
    print('orient',orient)
    input('press enter')
    
    i += 1
    print( 'i = ', i)
    
    #if time.time() > timeout:      
    #    break

import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import sys, time, math
import serial
import serial.tools.list_ports
#print([comport.device for comport in serial.tools.list_ports.comports()])

# configure the serial port
ser = serial.Serial(
    port='COM21',
    baudrate=9600,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_TWO,
    bytesize=serial.EIGHTBITS
)
ser.isOpen()

xsize=550
   
def data_gen():
    t = data_gen.t
    while True:
       t+=1
       strin = ser.readline()
       val =float(strin)
       yield t, val

def run(data):
    # update the data
    t,y = data
    if t>-1:
        xdata.append(t)
        ydata.append(y)
        if t>xsize: # Scroll to the left.
            ax.set_xlim(t-xsize, t)
        line.set_data(xdata, ydata)
    return line,

def on_close_figure(event):
    sys.exit(0)

data_gen.t = -1
fig = plt.figure()
fig.canvas.mpl_connect('close_event', on_close_figure)
ax = fig.add_subplot(111)
line, = ax.plot([], [], lw=2)
ax.set_ylim(0,300)
ax.set_xlim(0, xsize)
ax.grid()
xdata, ydata = [], []
# Add title and axis names
plt.title('Thermal Profile')
plt.xlabel('Time (s)')
plt.ylabel('Temperature (Celsius)')
    
# Important: Although blit=True makes graphing faster, we need blit=False to prevent
# spurious lines to appear when resizing the stripchart.
ani = animation.FuncAnimation(fig, run, data_gen, blit=False, interval=100, repeat=False)
plt.show()
import numpy as np
import matplotlib
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import sys, time, math
import serial
import serial.tools.list_ports
from time import sleep
#print([comport.device for comport in serial.tools.list_ports.comports()])

MIN_SOAK_TEMP = 120                                                                 # Minimum values for thermal profile parameters
MIN_SOAK_TIME = 30
MIN_REFLOW_TEMP = 200
MIN_REFLOW_TIME = 15

MAX_SOAK_TEMP = 180                                                                 # Maximum values for thermal profile parameters
MAX_SOAK_TIME = 90
MAX_REFLOW_TEMP = 240
MAX_REFLOW_TIME = 60

soakTemp = 0                                                                        # Declare thermal profile parameter variables
soakTime = 0
reflowTemp = 0
reflowTime = 0
default = "blah"

# configure the serial port
#ser = serial.Serial(
#    port='COM21',
#    baudrate=9600,
#    parity=serial.PARITY_NONE,
#    stopbits=serial.STOPBITS_TWO,
#    bytesize=serial.EIGHTBITS
#)
#ser.isOpen()

xsize=500
ysize=255
   
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

# Description:      1. Prompts user to enter the desired value for a given thermal profile parameter and checks user input for validity (within specfied range).
#                   2. Sends the value to the controller and checks to ensure that the controller has received the correct data.
# Parameters:       minimum - Minimum acceptable value
#                   maximum - Maximum acceptable value
# Returns:          The reflow parameter value if valid and successfully sent, 'False' otherwise.
def get_parameter(minimum, maximum):
    try:
        value = int(input("- "))
        #print(value)
    except(ValueError):                                                             # Determine whether input is an integer
        print("Error: not an integer")
        return False
    if(value < minimum or value > maximum):                                         # Determine whether input is within acceptable range
        print("Error: outside range")
        return False

    ser.write(str(value).encode())                                                  # Encode input as binary data and send to controller
    if(int(ser.readline()) != value):                                               # Check controller has received correct data
        print("Error: data not successfully received by controller")
        return False
    
    return value

# Main module
if(__name__ == "__main__"):
    try:
        ser = serial.Serial("COM21", 9600)                                           # Configure and open serial port
        if(ser.isOpen() == True):
            print("Connection to controller established")
    except(IOError):                                                                # Determine whether port has been successfully opened
        print("Error: connection to controller failed")
        quit()
    sleep(1)

    print()
    print("Do you want to use default values? (Yes/No)")
    while(default != "Yes" and default != "No" and default != 2 and default != 1):
        default = input("- ")
        if (default != "Yes" and default != "No"):
            print("Please enter Yes/No")
        if (default == "Yes"):
            default = 1
        if (default == "No"):
            default = 2
    ser.write(str(default).encode())
    if(int(ser.readline()) != default):                                               # Check controller has received correct data
        print("Error: data not successfully received by controller")

    if (default == 2 ):

        print()
        print("Enter soak temperature ("                                                # Display prompt and acceptable range
              + str(MIN_SOAK_TEMP) + "\u2103 - "
              + str(MAX_SOAK_TEMP) + "\u2103):")
        while(soakTemp == False):                                                       # Obtain and transmit desired soak temperature
            soakTemp = get_parameter(MIN_SOAK_TEMP, MAX_SOAK_TEMP)

        print()
        print("Enter soak time ("                                                       # Display prompt and acceptable range
              + str(MIN_SOAK_TIME) + "s - "
              + str(MAX_SOAK_TIME) + "s):")
        while(soakTime == False):                                                       # Obtain and transmit desired soak time
            soakTime = get_parameter(MIN_SOAK_TIME, MAX_SOAK_TIME)

        print()
        print("Enter reflow temperature ("                                              # Display prompt and acceptable range
              + str(MIN_REFLOW_TEMP) + "\u2103 - "
              + str(MAX_REFLOW_TEMP) + "\u2103):")
        while(reflowTemp == False):                                                     # Obtain and transmit desired reflow temperature
            reflowTemp = get_parameter(MIN_REFLOW_TEMP, MAX_REFLOW_TEMP)

        print()
        print("Enter reflow time ("                                                     # Display prompt and acceptable range
              + str(MIN_REFLOW_TIME) + "s - "
              + str(MAX_REFLOW_TIME) + "s):")
        while(reflowTime == False):                                                     # Obtain and transmit desired reflow time
            reflowTime = get_parameter(MIN_REFLOW_TIME, MAX_REFLOW_TIME)

    print()
    print("Press 'START' button to begin reflow cycle")
    while(True):                                                                       # Wait for start command to be received before ploting graph
        if(ser.readline() == b"Start\n"):
            print()
            print("Starting reflow cycle")
            break

def on_close_figure(event):
    sys.exit(0)

data_gen.t = -1
fig = plt.figure()
fig.canvas.mpl_connect('close_event', on_close_figure)
ax = fig.add_subplot(111)
line, = ax.plot([], [], lw=2)
ax.set_ylim(0,ysize)
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

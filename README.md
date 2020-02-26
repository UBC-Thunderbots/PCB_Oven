# Reflow Oven Controller

This was designed from a previous iteration of our reflow oven controller, redesigning it in Arduino to be maintainable. Our current setup is a perfboard arduino shield, where this repository stores the code for the project, PCB design and schematics, and Thermal profile graphs when reflowing PCB's using python to do the graphing.

The current system is shown below:

( insert picture )

One test graph is shown below, created with python:

![](Images/Reflow Graphs/Reflow_test_8.png)
![](Images/Reflow Graphs/Reflow_test_9.png)
![](Images/Reflow Graphs/Reflow_test_10.png)
![](Images/Reflow Graphs/Reflow_test_11.png)

# How to use the Controller

#### Step 1: Connections
- Plug in the negative and positive terminals for the thermocouple, marked on both the wire and the terminal connector
- Plug in the CTRL/GND/5V connection to the header, keeping in mind the orientation (Yellow/Black/Red)

#### Step 2: Arduino IDE
- Install the Arduino IDE, in case the code needs to be re-flashed onto the controller
- Plug in the USB Blaster cable to the Arduino, and check if you see the display on the LCD (currently sorta broken but this would be the case usually, with a good LCD)
- Re-flash the code if you aren't sure

#### Step 3: Run the Python Script or PuTTY on the correct COM port
- Python brings up a graph that plots the temperature vs time
- PuTTY will show you the temperature every second
- You can't use both at once (obviously)

#### Step 4: Start the Reflow Process
- Press the start button (shown in layout above)
- The process will abort automatically if the temperature does not rise fast enough (i.e. the thermocouple fell out of the oven)
- The process can be aborted by using the abort button (also shown above)
- The process will start the cooling cycle early if the temperature goes above 235 C

#### Step 5: Be patient and wait
- The controller will beep at you every time it changes state
- It will also give you a long beep when it is time to open the oven for cooling
- 6 beeps at the end means it is cool enough inside the oven for you to touch the PCB (usually)

#### Step 6: You're Done!!!!


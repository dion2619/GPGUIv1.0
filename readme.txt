GPGUI (General purpose graphical user interface)

This arduino library is for a custom pcb based around a ESP32 microcontroller and can be used as a graphical control system 
for any device over a UART port or i2c bus.
Also there is a basic raycasting engine that will render a map defined in two buffers(60x60 map grid and 16 16x16 wall textures).
Also has a basic real time 3D vertex and triangle mesh rendering pipeline.

PCB Hardware 
VGA PORT 48 COLORS 320X240 
PS2 KEYBOARD PORT
MIDI IN PORT
MIDI OUT PORT
I2C PORT
UART PORT 
CPU BUZZER

NOT YET SUPPORTED BUT WILL ALSO HAVE

PLAYSTATION 2 CONTROLLER PORT
PS2 MOUSE PORT
38KHZ IR REMOTE RECEIVER 
X4 UART MULTIPLEXED PORTS 

!! MUST USE THE ARDUINO ESP32 v2.0.0 CORE !! 
on some of the other cores the keyboard and cpu buzzer wont work..
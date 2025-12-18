# Cyber_physical_system_project
# Temperature and Motion-Based Fan control 

**Description**<br>
#In this project cyber physical system is implemented, where Sensor Node and edge device is exchang the data via UART communication protocol. The goal of the project is to established a close-loop system, where the sensor node send the environmental data to the edge device and the edge device process and make a autonomous control decision for the sensor node to control the actuators based on the temperature and motion detection.
DHT11 and PIR motion sensor and ATmega328P micro controller, fan and led were used in sensor node part and Raspberry pi 5 was used as a edge device.

Hardware requirement for operation
Power for ATmega328P = 5V ( power supply) edge device can supply it via usb cable 
Pir sensor = connect to INTO( PD2)
DHT11 sensor = connect to PD3
Fan = PB1
LED = PB5

The TX and RX communication established automatically when we connect the usb cable between Raspberry pi and Adruino UNO. 

Once all the pin is connected according to the above explain
upload the code in Adruino IDE for sensor node and open the python code in Thonny IDE in raspberry pi. 

In the sensor node the system will read the temperature every 3 second. It also monitor the motion using PIR sensor via an external interrupt. If the temperature is more than 20 and motion get detected the edge device send command message back to sensor node to control the fan and led accordingly.  







 


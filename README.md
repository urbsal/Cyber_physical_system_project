# Cyber_physical_system_project
# Temperature and Motion-Based Fan control 

**Description**<br>
In this project cyber physical system was implemented, where Sensor Node and edge device is exchang the data via UART communication protocol. The goal of the project is to established a close-loop system, where the sensor node send the environmental and motion data to the edge device and the edge device process and make a autonomous control decision for the sensor node to control the actuators based on the temperature and motion detection.<br>
DHT11 and PIR motion sensor and ATmega328P micro controller, fan and led were used in sensor node part and Raspberry pi 5 was used as a edge device.<br>

Hardware requirement for operation<br>
Power for ATmega328P = 5V ( power supply) edge device can supply it via usb cable <br>
Pir sensor = connect to INTO( PD2)<br>
DHT11 sensor = connect to PD3<br>
Fan = PB1<br>
LED = PB5<br>

The TX and RX communication established automatically when we connect the usb cable between Raspberry pi and Adruino UNO. <br>

Once all the sensor are connected to respective pin following the above mentioned instruction,
upload the code in Adruino IDE for sensor node and open the python code in Thonny IDE in raspberry pi. <br>

In the sensor node the system micro controller ( UNO) will read the temperature every 3 second. It also monitor the motion using PIR sensor via an external interrupt. If the temperature is more than 20 and motion get detected the edge device send command message back to sensor node to control the fan and led accordingly.  







 


# npsystem
Distributed control system

![image](https://user-images.githubusercontent.com/86890989/124366151-32a67500-dc56-11eb-9b3a-fc1357b78c66.png)

I created this to control various things in my house (pumps, well, valves, automatic plant watering, etc...). The goal was to create scalable, cheap, reliable system working 24/365 without reboot, where all controllers are independent and do not require main server to run automation algorithms. The system is built on RS-485 with signal loss detection. The communication protocol is based on set of protocols utilizing virtual marker. Each controller has I2C interface for additional modules with custom software (like 1-wire devices, or any other). The bridge between RS-485 and ethernet is built on enc28j60 with atmega8, the most expensive part is max3082 transivers (2 pieces for controlles and bridge), the controllers could be either atmega8 or atmega16, in future I'm planning to add stm32 chips. 

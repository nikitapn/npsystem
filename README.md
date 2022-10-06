# npsystem

I created this to control various things in my house (pumps, well, valves, automatic plant watering, etc...). The goal was to create scalable, cheap, reliable system working 24/365 without reboot, where all controllers are independent and do not require main server to run logic algorithms. The system is built on RS-485 with signal loss detection. The communication protocol is based on set of protocols utilizing virtual marker. Each controller has I2C interface for additional modules with custom software (like 1-wire devices, or any other). The bridge between RS-485 and ethernet is built on enc28j60 with atmega8, the most expensive part is max3082 transivers (2 pieces for controlles and bridge), the controllers could be either atmega8 or atmega16, in future I'm planning to add stm32 chips. \
\
I also want to rewrite it all for llvm backend, but I'm extremely busy right now...
\
\
![image](https://user-images.githubusercontent.com/86890989/187889033-cefc38d6-a4a2-458e-8b0a-cc21c07e97b0.png)
<p align="center">
  <b>Structural Diagram</b>
  <br/><br/>
  <img src="https://user-images.githubusercontent.com/86890989/194228189-b999a2b3-aa35-41a7-9b53-975eb641f8f7.png">
</p>

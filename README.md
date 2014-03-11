ArduScope
=========

ArduScope is a litle base for use the an Arduino board as data adquisition board and a HTML5 web browser as visualizer.

#Details
* Websocket communication between Arduino board and web browser.
* Maxium sampling frequency about 1.3 kHz.
* Boostrap style and javascript for GUI.
* D3 Drawing engine for graph.

#Requirements
* Arduino Ethernet (Arduino Ethernet shield have not been test).
* Signal generator between 0 and 5 v output voltage (a pontentiometer or whatever sensor can be used also).
* Linux Host (I used Ubuntu 12.04).
* GNU Make.
* Arduino core, AVR compiler (gcc-avr 4.5.3) and loaders.
* Free ethernet interface and cable.
* A web browser (it has been tested with Google Chrome/ium and Firefox)

# Intructions (Ubuntu)#
1. Download the project code (zip or using git clone)
2. Change directory to Firmware:
```sh
cd ArduScope/Firmware
```
3. Compile code using make:
```sh
make
```
4. Connect board to host computer. Check if your Arduino board is connected to /dev/ttyACM0 (default defined in the Makefile). In case of being different change it (line 9 in Makefile file).

5. Upload code to the board using the following command:
```sh
make load
```

6. Connect the Arduino board to the host computer using an Ethernet Cable.

7. Due the fact that the IP address by default of the Arduino is 192.168.10.2 you have to change the IP address of your interface (eth0 or whatever interface). You can use the following command:
```sh
sudo ifconfig eth0 192.168.10.1 netmask 255.255.255.0
```

8. Open with a web browser the Graphical Interface (It is not stored in the Arduino board SD yet).

```sh
firefox ArduScop/GUI/index.html 
```

9. The should see something like the following screenshoot:
![alt text](https://github.com/xarteaga/ArduScope/blob/master/screenshots/pwmGraph.png?raw=true "Waveform Screenshot")

10. Hack and enjoy ;)


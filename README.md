# HeartSwitch

www.heartswitch.eu
Heartbeat potential for Smart Home Technology

HeartSwitch BETA is a project that allows via an MCU connected to the internet (like NodeMcu) to monitor the heartrate by IoT (in this version via Blynk) and select the triggering of a home automation event through a programmable event related to the BPM (beats per minute of the user), as above the threshold, below the threshold, or at a certain frequency range.
It connects directly to download the exact time from NTP. It does not necessary a backup battery.

HeartSwitch aims to build a completely intelligent home automation system based on signals from bracelets. As a goal, we refer mainly to nursing homes, hotels and hospitals, but also to solutions for private homes. 

This project has been tested using:
- Nodemcu v.3 (Wifi SoC ESP8266);
- MAX30100 or MAX30102 or MAX30105;
- OLED Display 1306 (Warning! Please check that yours has the GND pin on the right);
- Blynk IoT App;
- Optional: passive buzzer + a led;

Blynk app lets you control any hardware connected to Digital and Analog pins without having to write any additional code.
Lets you control any hardware connected to Digital and Analog pins without having to write any additional code.

For any custimization, this is the Virtual Pin configuration:

V0 > Heartbeat dashboard gauge

V1 > Event Type ( (1)Over|(2)Under|(3)Range[5+1+5] )

V2 > BPM setting

V3 > Action setting

V4 > On-line | Hold (Off-line)

V5 > Time range scheduler

V6 > Status virtual led (if the socket is ON/OFF)

V7 > N.C.

V8 > Selected Range label (visible just for V3=3)

Please check that you have installed all the required libraries before uploading the sketch.

Before starting to load the code on your NodeMcu, remember to SEARCH and EDIT these params:
/* *********************************** */
/*   SETUP WI-FI   */
char ssid[] = "[SSID]";
char pass[] = "[PaSsW0rD]";
/* ON/OFF Button 433mhz DECIMAL Decode */
const long setON = 9327618;
const long setOFF = 9327617;
/*  **********************************  */


Right! Configure your SSID and password WEP / WPA / WPA2 here.
It can currently control an electrical outlet compatible with 433Mhz transmissions.
You probably don't yet know the communication code of the remote control it wants to emulate. We ask you to use a sniffer for this. You can contact us if you need help. We will try to make the library compatible with your device.

Support our mission on Kickstarter page and allow us to carry out this project. You will receive the exclusive home automation kit for the preview.

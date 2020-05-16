# HeartSwitch

www.heartswitch.eu
Heartbeat potential for Smart Home Technology

HeartSwitch BETA is a project that allows via an MCU connected to the internet (like NodeMcu) to monitor the heartrate by IoT (in this version via Blynk) and select the triggering of a home automation event through a programmable event related to the BPM (beats per minute of the user), as above the threshold, below the threshold, or at a certain frequency range.
It connects directly to download the exact time from NTP. It does not necessary a backup battery. You can setup the NTP server from the configuration variabiles.

You can use the MAX10300 / MAX 10302 / MAX10305 heart rate reader. They all work. We used the MAX10302 because some MAX10300s have design errors and need additional resistors. The MAX10305, on the other hand, has extra features that may not be necessary at this time.

The device has two LEDs, one emitting red light, another emitting infrared light. For the pulse rate, only infrared light is needed. Both red and infrared light are used to measure oxygen levels in the blood.

When the heart pumps blood, there is an increase in oxygenated blood due to the presence of more blood. As the heart relaxes, the volume of oxygenated blood also decreases. Knowing the time between the rise and fall of oxygenated blood, the pulse rate is determined.
So that oxygenated blood absorbs more infrared light and passes more red light while deoxygenated blood absorbs red light and passes more infrared light. This is the main function of the MAX30102: it reads the absorption levels for both light sources and stores them in a buffer that can be read via I2C.

We have added some formulas (hacks) to optimize very high gaps and false positives.

HeartSwitch aims to build a completely intelligent home automation system based on signals from bracelets. As a goal, we refer mainly to nursing homes, hotels and hospitals, but also to solutions for private homes. 

Bill of materials.

This project has been tested using:

- Nodemcu v.3 (Wifi SoC ESP8266);

Reference NodeMCU: https://nodemcu.readthedocs.io/en/master/

- MAX30100 or MAX30102 or MAX30105;

Reference Heart Pulse sensor: https://datasheets.maximintegrated.com/en/ds/MAX30100.pdf

- Generic RF TX (like FS1000A);

Reference Generic RFSwitch: https://github.com/sui77/rc-switch

- OLED Display 1306 (Warning! Please check that yours has the GND pin on the right);

Reference Display: https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf

- Blynk IoT App;

Reference BLYNK: https://github.com/blynkkk/blynk-library/releases/latest

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

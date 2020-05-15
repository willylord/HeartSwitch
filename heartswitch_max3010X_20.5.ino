/******************************************
// HEARTSWITCH V.020.5 (blob master from 05.2020)
// --- automates our actions ---
// AUTHORS: C.Cipresso - A.Verde , 2016-2020 
// DEVICE: Lolin NodeMCU V3 + max30100 heartbeat + Display OLED SSD1306 128x64 (16pxYellow+48pxCyan) + GPIO 2 buttons remote control
// Reference NodeMCU: https://nodemcu.readthedocs.io/en/master/
// Reference Heart Pulse sensor: https://datasheets.maximintegrated.com/en/ds/MAX30100.pdf
// Reference Display: https://cdn-shop.adafruit.com/datasheets/SSD1306.pdf
// Reference Generic RFSwitch: https://github.com/sui77/rc-switch
// Reference BLYNK: https://github.com/blynkkk/blynk-library/releases/latest

// Download and build your shield here: http://www.heartswitch.eu/index.php/pre-order/developers.html
 ******************************************/
#include <Wire.h> // I2C Library

#include <ESP8266WiFi.h> // ESP WiFi


/* INCLUDE LIBRARIES */
#define BLYNK_PRINT Serial
#include <Blynk.h>
#include <BlynkSimpleEsp8266.h> // Blynk
#include <OakOLED.h> // Display 

/* Pulse sensor */
#include <MAX30105.h>
#include <heartRate.h>

/* Remote Switch 433Mhz */
#include <RCSwitch.h>
RCSwitch heartSwitch = RCSwitch(); // Define the heartswitch remote controller

// calling NTPClient library
#include <NTPClient.h>
#include <WiFiUdp.h>


#define REPORTING_PERIOD_MS 1000 // this for shoot the data online every second

// Initilize Display
OakOLED oled; // Connections for OLED Display : SCL PIN - D1 , SDA PIN - D2 , INT PIN - D0

// Initilize Pox
MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

/* Setup a phisical PIN for trigger button */
const int statusPin = D5;

/* Global variable for state of the time range : True if time in range, False if not */
bool DateInRange;

/* Local variables for taking data VIRTUAL VAR application
 *  arduino store time range when receiving from app blynk
 *  the data will be received when user change information in the application.
 *  if reset accurs or power off power on, this information will reset and losed, so you can think in stocking data in eeprom and load it in starting up
 */

// TIME INT DECLALATION
// start stop seconds
int start_second;
int stop_second;

// start stop minutes
int start_minute;
int stop_minute;

// start stop hours
int start_hour;
int stop_hour;
/*
 * id day index i is selected, days[index] = 1 else 0
 * the first day is monday, days[0] -> monday days[6] -> sunday
 */
int days[7];

String wd[7]={"SUN","MON","TUE'","WED","THU","FRI","SAT"}; // name of weekdays
String mo[12]={"GEN","FEB","MAR","APR","MAY","JUN","JUL","AUG","SEP","OCT","NOV","DEC"}; // name of months


/* Definisco costanti */
#define heartratePin A0 // Define the default pin for heart connection (A0)
#define buzzer D6 // D6 Piezo buzzer
#define radio D7 // D7 Remote radio TX at 433mhz
#define feedback V6 // V6 used for feedback led
WidgetLED socket(feedback);
      
BlynkTimer timer;  // Define the timer using the BlynkTimer library

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "lkeO7sOGGaGbufgKl5Uq7FDRgR9hiZ3U";

/* *********************************** */
/*   SETUP WI-FI   */
char ssid[] = "[SSID]";
char pass[] = "[PaSsW0rD]";

/* ON/OFF Button 433mhz DECIMAL Decode */
const long setON = 9327618;
const long setOFF = 9327617;
/*  **********************************  */


// DST starts the last Sunday of March and ends the last Sunday of October
#define TIME_OFFSET 1       // local time offset during Standard Time
#define TIME_OFFSET_DST 2   // local time offset during DST 

const long utcOffsetInSeconds = TIME_OFFSET_DST*60*60; // time zone in seconds
const char* NTP_Server= "it.pool.ntp.org"; // your NTP server
const long UpdateTime = 60000; // interval to recheck time from NTP server in Milliseconds, her 10 minute

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_Server, utcOffsetInSeconds, UpdateTime);

String stringRange, infoAction, watchdog;
boolean btnState, socketState; // Bool global values

float beatRT, BPM, lastBPM; // Float global values

int range=5; // Setup the default range (es.: 5 = (5+1+5)=11 gap)

int s=0, m=1, h=12, d=1, timeEvent, from, to, edge, eventType, activator, enabled, peak, minBase, maxBase; // Other Int values

uint32_t tsLastReport = 0;

// INTRO Bitmap
// Parte destra, 45x48px
const unsigned char splash1 [] PROGMEM = {
  0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x7f, 
  0xfe, 0x00, 0x00, 0x00, 0x00, 0xf8, 0x07, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x01, 0x80, 0x00, 0x00, 
  0x03, 0x80, 0xc0, 0x80, 0x00, 0x00, 0x03, 0x0f, 0xf8, 0x40, 0x00, 0x00, 0x06, 0x3f, 0xfe, 0x20, 
  0x00, 0x00, 0x04, 0x7f, 0xff, 0x20, 0x00, 0x7f, 0x84, 0xe7, 0xff, 0x80, 0x01, 0xe1, 0xe5, 0xe7, 
  0xff, 0xc0, 0x03, 0x00, 0x31, 0xe7, 0xff, 0xc0, 0x04, 0x00, 0x1b, 0xc7, 0xff, 0xe0, 0x08, 0x3f, 
  0x8b, 0xc7, 0xff, 0xe0, 0x10, 0xff, 0xe3, 0xc7, 0xff, 0xf0, 0x03, 0xff, 0xf3, 0xc3, 0xff, 0xf0, 
  0x27, 0xff, 0xfb, 0xc3, 0xff, 0xf0, 0x07, 0xff, 0xff, 0xc3, 0xff, 0xf0, 0x0f, 0xff, 0xff, 0xc3, 
  0xff, 0xf0, 0x1f, 0xff, 0xff, 0xc3, 0xff, 0xf0, 0x1f, 0xff, 0xff, 0xc3, 0xff, 0xf8, 0x1f, 0xff, 
  0xff, 0x83, 0xff, 0xf8, 0x3f, 0xff, 0xff, 0x91, 0xff, 0xf0, 0x3f, 0xff, 0xff, 0x99, 0xff, 0xf0, 
  0x3f, 0xff, 0xff, 0x99, 0xff, 0xf0, 0xff, 0xff, 0xe7, 0x99, 0xff, 0xf8, 0xff, 0xff, 0xe7, 0x99, 
  0xff, 0xf8, 0x00, 0x03, 0xc7, 0x98, 0x00, 0x00, 0x00, 0x01, 0xc3, 0x98, 0x00, 0x00, 0x1f, 0xf9, 
  0x83, 0x9c, 0x00, 0x00, 0x0f, 0xf9, 0x93, 0x9f, 0xff, 0xe0, 0x0f, 0xf9, 0x11, 0x1f, 0xff, 0xe0, 
  0x07, 0xf8, 0x31, 0x1f, 0xff, 0xc0, 0x03, 0xf8, 0x39, 0x3f, 0xff, 0xc0, 0x03, 0xfc, 0x79, 0x3f, 
  0xff, 0xc0, 0x01, 0xfc, 0x78, 0x3f, 0xff, 0x80, 0x00, 0xfc, 0xfc, 0x3f, 0xff, 0x80, 0x00, 0x7f, 
  0xfc, 0x3f, 0xff, 0x00, 0x00, 0x3f, 0xfc, 0x3f, 0xff, 0x00, 0x00, 0x1f, 0xfc, 0x3f, 0xfe, 0x00, 
  0x00, 0x0f, 0xfe, 0x3f, 0xfe, 0x00, 0x00, 0x03, 0xfe, 0x3f, 0xfc, 0x00, 0x00, 0x01, 0xfe, 0x7f, 
  0xfc, 0x00, 0x00, 0x00, 0x7f, 0x7f, 0xf8, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xf0, 0x00, 0x00, 0x00, 
  0x07, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xc0, 0x00
};
// Parte sinistra, 82x48px
const unsigned char splash2 [] PROGMEM = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1d, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x09, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x09, 
  0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x0c, 0x6f, 0x8c, 0x7c, 0xfc, 0x73, 0x33, 0x6f, 0xdc, 0xc6, 0x00, 
  0x0c, 0x6c, 0x0c, 0x66, 0x30, 0xcb, 0x33, 0x63, 0x32, 0xc6, 0x00, 0x0c, 0x6c, 0x1e, 0x66, 0x30, 
  0xc1, 0xb6, 0x63, 0x30, 0xc6, 0x00, 0x0f, 0xef, 0x92, 0x7c, 0x30, 0xf1, 0xb6, 0x63, 0x30, 0xfe, 
  0x00, 0x0c, 0x6c, 0x33, 0x6c, 0x30, 0x79, 0xb6, 0x63, 0x30, 0xc6, 0x00, 0x0c, 0x6c, 0x3f, 0x66, 
  0x30, 0x18, 0xcc, 0x63, 0x30, 0xc6, 0x00, 0x0c, 0x6c, 0x33, 0x66, 0x30, 0x98, 0xcc, 0x63, 0x32, 
  0xc6, 0x00, 0x0c, 0x6f, 0xb3, 0x66, 0x30, 0x70, 0xcc, 0x63, 0x1c, 0xc6, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0xff, 0xff, 
  0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// The showSwitch () function returns the given "switch on/off" then: string 0/1
String showSwitch(boolean bol) {

  String caption="";
 
        if (bol==true)  {
        caption= caption+((char)15); // Aggiunto simbolo ON
        } else {
        caption= caption+((char)9); // Aggiungo simbolo OFF
        }
        return caption; // restituisce label
}

// Suono buzzer
void bebee(int beep, int mix) // Where "mix" indicates a variable multiplier
{
  tone(buzzer, beep); delay(50);      
  noTone(buzzer); delay(50);  
  tone(buzzer, beep*mix);  delay(75);      
  noTone(buzzer);   
    
}

void setup()
{ 

delay(100);
  
    // Define Output
    pinMode(statusPin, OUTPUT); // Use this virtual status Pin to check the HeartSwitch status
    pinMode(buzzer, OUTPUT); // Beeeeb and/or led
    pinMode(radio, OUTPUT); // 433 Mhz radio tx PIN
    pinMode(16, OUTPUT); // Heartbeat Interrupt
 
    // Debug console
    Serial.begin(115200);

    // **** RADIO REMOTE CONFIG ****
    // Transmitter is connected to correct PIN 
    heartSwitch.enableTransmit(radio);
    
    // Set pulse length 1 sec.
    heartSwitch.setPulseLength(1000);
    
    // Set number of transmission tries.
    heartSwitch.setRepeatTransmit(4);

    // **** INTRO BUMPER ****
    oled.begin(); // Initilize
    oled.clearDisplay(); // Clean the display
    delay(100); // wait 100 ms before start
    oled.drawBitmap( 0, 16, splash1, 45, 48, 1); // Carica Splash screen parte sinistra
    oled.display();
    
    // Vector line animation
    // loop from the lowest pin to the highest:
    for (int coordX = 45; coordX < 123; coordX = coordX+5) {
      oled.drawPixel( coordX, 41, 1); 
      oled.drawPixel( coordX, 42, 1); 
      oled.drawPixel( coordX+1, 41, 1); 
      oled.drawPixel( coordX+1, 42, 1); 
      oled.drawPixel( coordX+2, 41, 1); 
      oled.drawPixel( coordX+2, 42, 1); 
      oled.drawPixel( coordX+3, 41, 1); 
      oled.drawPixel( coordX+3, 42, 1);
      oled.drawPixel( coordX+4, 41, 1); 
      oled.drawPixel( coordX+4, 42, 1);         
      oled.display();
      delay(10);
     }
    
    oled.drawBitmap( 46, 16, splash2, 82, 48, 1); // Load left slash intro bmp
    bebee(1915,1.2);
    delay(100);
    oled.display();
 
    oled.setCursor(0,0);
    oled.print("automates our actions");
    oled.display();
         
    delay(4000); // Per 4 sec. circa
    oled.clearDisplay(); // Re-Clean

    // Wait connection wifi + Heartbeat
    oled.setTextSize(1);
    oled.setTextColor(1);
    oled.setCursor(19, 0);
    oled.print("CONNECTING ... ");
    oled.display();
   
  Blynk.begin(auth, ssid, pass); // Server / Wi-Fi auth 
  
  Blynk.syncAll(); // Update all values on the App

  timeClient.begin(); // Start the NTP Service

    // Initialize sensor code
    if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) // Use default I2C port, 400kHz speed
    {
      Serial.println("MAX30105 was not found. Please check wiring/power. ");
      while (1);
    }
    Serial.println("Place your index finger on the sensor with steady pressure.");
  
    particleSensor.setup(); // Configure sensor with default settings
    particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
    particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED
     
  timer.setInterval(2000L, sendUptime); // Set send uptime
  
}

void switchAction(boolean action, boolean switching) {
    /* Switch Action Usage:
     * HEARTSWITCH CONTROLLER
     * ITA:
     * Action=0 > Azione di default settata su switch off
     * Action=1 > Azione di default settata su switch on
     * La variabile Action è valorizzata dinamica per questa funzione
     * 
     * Switching=1 > Il controller deve compiere l'azione presente in Action
     * Switching=0 > Il controller deve compiere l'opposto dell'azione presente in Action
     * La variabile Switching è valorizzata statica per questa funzione, direttamente dallo switch-case
     * 
     * Nel mio caso turnON deve verificarsi solo se action (cioè l'azione di default) e
     * switching (cioè se l'evento si è verificato) sono entrambi =TRUE o =FALSE, altrimenti vai su turnOFF
       * ENG:
       * Action = 0> Default action set to switch off
       * Action = 1> Default action set on switch on
       * The Action variable is dynamically enhanced for this function
       *
       * Switching = 1> The controller must perform the action present in Action
       * Switching = 0> The controller must do the opposite of the action in Action
       * The Switching variable is static value for this function, directly from the switch-case
       *
       * In my case turnON should only occur if action (i.e. the default action) e
       * switching (i.e. if the event occurred) are both = TRUE or = FALSE, otherwise go to turnOFF
       * /
      */
 
    if (action == switching) { // LOGIC: If the 2 variables are both TRUE or FALSE
       turnON();
      } else { 
       turnOFF();
      } 

} 

// turn > ON funcition
void turnON() {
  
  if (!socketState) { // Check that they are not already ON
      heartSwitch.send(setON, 24); // ON
      bebee(1432,1);
  }
  Blynk.setProperty(feedback, "color", "#32A852"); // Led feedback update Red
  socketState=true; // Register as ON
  
}

// turn > OFF funcition
void turnOFF() {
  
  if (socketState) { // Check that they are not already OFF
      heartSwitch.send(setOFF, 24); // OFF
      bebee(1014,1);
  }
  Blynk.setProperty(feedback, "color", "#D3435C");  // Led Feedback update Green
  socketState=false; // Register as OFF
  
}


/***VIRTUAL VAR***/ 
BLYNK_WRITE(V1)  { // V1= Event Type
  
  String stringNull;
  stringNull = " ";

   // Parameterization of the threshold type
   // 1: Above the threshold
   // 2: Below the threshold
   // 3: Within the range +/- 4 units (therefore 7 values in all). !! This parameter is editable in global variables


  eventType=param.asInt(); // This register the default 'eventType' from the app GUI

  /* In questo spazio si effettuato le valorizzazioni delle azioni da svolgere */ 
  switch (param.asInt()) // Switchcase for 'eventType'
      {
      case 1: { // Over 
      Blynk.virtualWrite(V8, stringNull);
      break;
      }
      case 2: { // Under
      Blynk.virtualWrite(V8, stringNull);
      break;
      }
      case 3: { // Range
      Blynk.virtualWrite(V8, stringRange);
      break;
      }
    }
    
 }

/***VIRTUAL VAR***/ 
BLYNK_WRITE(V2) { // V2= Actions to be performed on BPM value (range parameter update)

    edge=param.asInt(); // Check threshold selected to manage it
  
    from=(edge-range);
    to=(edge+range);
    
  // Send string to app
  stringRange=(String("▶")+from+String("〰")+to+String("◀"));
 
  if (eventType == 3 ) { // I update the caption of the range only if the value of V1 = Range (4)
  Blynk.virtualWrite(V8, stringRange);
  }
  
}

/***VIRTUAL VAR***/ 
BLYNK_WRITE(V3) { activator=param.asInt(); }  // V3= Type of action ON / OFF
/***VIRTUAL VAR***/ 
BLYNK_WRITE(V4) { enabled=param.asInt(); } // V4= Enabled or Disabled
/***VIRTUAL VAR***/ 
BLYNK_WRITE(V5) { // V5= Time Scheduler widget 
  TimeInputParam t(param);

  // Process start time
  if (t.hasStartTime())
    {
   start_second = t.getStartSecond();
   start_minute = t.getStartMinute();
   start_hour = t.getStartHour();
    }
  if (t.hasStopTime())
    {
     stop_second = t.getStopSecond(); 
     stop_minute = t.getStopMinute();
     stop_hour = t.getStopHour();
    }
  // Process weekdays (1. Mon, 2. Tue, 3. Wed, ...) and change it into (1. Sun 2. Mon 3.Tue...)

  for (int i = 1; i < 7; i++) {
    if (t.isWeekdaySelected(i)) {
      days[i] = 1;
    }
    else days[i] = 0;
  }
  if (t.isWeekdaySelected(7)) {
      days[0] = 1;
    }
    else days[0] = 0;
}


/* TIME CLOCK
 * ********
 * this is an auto update function need to be called in loop()
 * it is automatically update the system time in NTP library, and automaticaly update time from NTP server every IntervalTime
 * it is not blocked function, it make quick process
 */

long time_now; // used for comparison time in every second

void time_tick(void)
{
  if(millis() - time_now >= 1000)
  {
    time_now = millis();
    timeClient.update();
    
    
    if(days[timeClient.getDay()])
    {
      long  date_config = 3600 * timeClient.getHours() + 60 * timeClient.getMinutes() + timeClient.getSeconds();
      long data_start = 3600 * start_hour + 60 * start_minute + start_second;
      long date_stop = 3600 * stop_hour + 60 * stop_minute + stop_second;
      if (date_config < date_stop && date_config > data_start) DateInRange = true;
      else DateInRange = false;
    }
    else  DateInRange = false;
    
  }
}


/* TIME CLOCK every SECONDs
 * ********
 * this function increment time every second,
 * every second need to be called to auto update time
 * seconds increment + 1
 * when second = 59 -> second = 0 and minute +1
 * when minute = 59 -> minute = 0 and hours +1
 * when hours =23 -> hours = 0 and days +1
 * when days = 6 -> days = 0
 * this function test also if the current time is in right range or not
 * if time in range : DateInRange = true
 * if not : DateInRange = false
 * DateInRange is a real time variable
 */
void add_time_second(void) {
  if(s<59) s++;
  else
  {
    s=0;
    if(m<59) m++;
    else
    {
      m=0;
      if(h<23) h++;
      else
      {
        h=0;
        if(d<6) d++;
        else d = 0;
      }
    }
  }
  DateInRange = false;
  if(days[d])
  {
    long  date_config = 3600 * h + 60 * m + s;
    long data_start = 3600 * start_hour + 60 * start_minute + start_second;
    long date_stop = 3600 * stop_hour + 60 * stop_minute + stop_second;
    if (date_config < date_stop && date_config > data_start) DateInRange = true;
  }
}

// ******pox
// sendUpTime() in questa funzione indico un intervallo ulteriore

void sendUptime() { 

    delay(50);
 
} 
  


void loop() {

  Blynk.run();  // Default RUN
  timer.run();  // Initiates BlynkTimer
  time_tick(); // Autoupdate current time

  btnState=digitalRead(statusPin); // Registro lo stato iniziale dello switch



  // **********
  // Pulse sensor code
  // **********
  long irValue = particleSensor.getIR();
  if (checkForBeat(irValue) == true)  {
    
    // We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatRT = 60 / (delta / 1000.0);

    peak=10; // define a peak
    minBase=54; // define a tolerable MIN
    maxBase=170; // define a tolerable MAX
    
    if (beatRT < 255 && beatRT > 20)  {
      rates[rateSpot++] = (byte)beatRT; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable

      //Take average of basics BPM readings
      BPM = 0;
      for (byte x = 0 ; x < RATE_SIZE ; x++)
        BPM  += rates[x];
        BPM  /= RATE_SIZE;

      // *******
      // False positive hacks
      // *******
      if (BPM<minBase) BPM=0; // If BPM is too low then cancel the value
      if (BPM>maxBase) BPM=0; // If BPM is too high then cancel the value
      if (BPM==0) BPM=lastBPM; // If BPM returns to 0 it still shows the last value
     if (BPM >(lastBPM+peak) && (lastBPM!=0)) BPM=lastBPM+(random(2,3)); // If the newly detected BPM increases the previous element by quite a bit, then it increases by steps
     if (BPM <(lastBPM-peak) && (lastBPM!=0)) BPM=lastBPM-(random(2,3));  // If the newly detected BPM decrements a lot earlier, then decrease by steps
    
        lastBPM=BPM; // Mantengo in memoria l'ultimo valore
        
    }
 
/**********************/
/***END SENSOR CODE ***/
 
  }

  // PRINT REPORT ON SERIAL AND SCREEN
  if (millis() - tsLastReport > REPORTING_PERIOD_MS)   {

        Serial.print("Time: ");
        Serial.print(timeClient.getHours());
        Serial.print(":");
        Serial.print(timeClient.getMinutes());
        Serial.print(" - Time-range App: ");
        Serial.println(DateInRange);
    
        Serial.print("Heart rate: ");
        Serial.print(BPM);
//        Serial.print(" bpm / SpO2: ");
//        Serial.print(SpO2);
//        Serial.print(" % - ");
        Serial.print("Edge: ");
        Serial.print(edge);
        Serial.print(" - Default Action (on/off): ");
        Serial.print(activator);
        Serial.print(" - Event type: ");
        Serial.print(eventType);
        Serial.print(" - (Scheduler: ");
        Serial.print(timeEvent);
        Serial.print(" - Enabled: ");
        Serial.print(enabled);
        Serial.print(") - Event On-the-range: ");
        Serial.println(btnState);

        Blynk.virtualWrite(V0, BPM); // V0=BPM
        Blynk.virtualWrite(V10, btnState); // V10=Gfx Feedback State
 //        Blynk.virtualWrite(V9, SpO2); // V9=SpO2


        // Here 3 events Switch-case : OVER, UNDER, RANGE.
        // TRIGGERS ***

        if (enabled && DateInRange) {

            //  PRINT infoAction string
            infoAction="POWER ";
            
            if (activator == 1) { 
              infoAction+="ON  @ ";
            } else { 
              infoAction+="OFF @ ";
            } 
       
     
          switch (eventType) { // Switch according to the type of event (drop-down menu V1)
              
                case 1: { // Over 
                  if (BPM>=edge) { 
                        Serial.print("It's OVER the "); // if BPM is above the threshold
                        Serial.println(edge);
                        btnState=true;
                        switchAction(activator,true); 
                        } else { 
                        btnState=false;
                        switchAction(activator,false); 
                      }
                        infoAction+=(char)30;
                        infoAction+=edge;
                        infoAction+=" BPM";
                break;
                }
                
                case 2: { // Under
                  if (BPM<=edge) {            
                        Serial.print("It's UNDER the "); // if BPM is under the threshold
                        Serial.println(edge);
                        btnState=true;
                        switchAction(activator,true); 
                        } else { 
                        btnState=false;
                        switchAction(activator,false); 
                      } 
                        infoAction+=(char)31;
                        infoAction+=edge;
                        infoAction+=" BPM";
                break;
                }
                
                case 3: { // Range
                  if ((BPM>=from && BPM<=to)) { // if BPM is into the range
                        Serial.print("It's INNER the ");
                        Serial.println(edge);
                        btnState=true;
                        switchAction(activator,true); 
                        } else { 
                        btnState=false;
                        switchAction(activator,false); 
                    }  
                        infoAction+=(char)16;
                        infoAction+=from;
                        infoAction+=(char)126;
                        infoAction+=to;
                        infoAction+=(char)17;
                break;
                }
          
          } // End switch case
              
        } else {
             
             infoAction="NO SCHEDULED ACTIONS";
             }  // End enabled case

        digitalWrite(statusPin, btnState); // Update Pin status
        oled.clearDisplay();

        // PRINT TIME
         oled.setTextSize(1);
         oled.setCursor(2, 5);
        if (timeClient.getHours()<10) oled.print("0");
         oled.print(timeClient.getHours());
         oled.print(":");
         if (timeClient.getMinutes()<10) oled.print("0");
         oled.print(timeClient.getMinutes());  
         oled.print(" ");
    //     oled.print(wd[timeClient.getDay()]);  
         oled.print("HeartSwitch");


       // I write a "please wait" if beat detected but BPM still = 0
       if ((irValue > 50000) && (BPM==0)) {
        oled.setTextSize(1);
        oled.setTextColor(1);
        oled.setCursor(3, 27);
        oled.print("...");
   
        } 
            
      // Heart Icon
       if ((irValue > 50000) && (BPM>0))  {
        oled.setTextSize(2);
        oled.setTextColor(1);
        oled.setCursor(8, 25);
        oled.print((char)3);
        } else {
        BPM=0; // force reset the BPM value
       }
         
        // Switchstate
        oled.setTextSize(2);
        oled.setTextColor(1);
        oled.setCursor(114, 0);
        oled.println(showSwitch(socketState));

      oled.drawFastHLine( 0, 17, 128, 1);
   

         // BPM Label
        oled.setTextSize(2);
        oled.setTextColor(1);
        oled.setCursor(28, 26);
        oled.print("BPM:");
        
        // BPM Value
        oled.setTextSize(2);
        oled.setTextColor(1);
        oled.setCursor(75, 26);
        
        // if different from ZERO, the wristband is weared
        
        if (BPM !=0) {     
          oled.println(BPM,0);
          } else {
           oled.println("**"); // Else print some **
          }
        
/*        // SpO2 Label
        oled.setTextSize(2);
        oled.setTextColor(1);
        oled.setCursor(1, 47);
        oled.println("SpO2:");

        // SpO2 Value
        oled.setTextSize(2);
        oled.setTextColor(1);
        oled.setCursor(61, 47);
      //  oled.println(pox.getSpO2());
        oled.println(SpO2,1);
*/       
   
      oled.drawFastHLine( 0, 47, 128, 1);
                        
        oled.setTextSize(1);
        oled.setCursor(2, 55);     
        oled.print(infoAction);
        
        oled.display();

      tsLastReport = millis();

    }

}

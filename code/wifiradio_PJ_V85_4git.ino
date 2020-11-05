

/*
 * This sketch is for RETRORADIO WIFI-PILOT 
 * 
 * Upgrade dial tuned vintage receiver with internet web radio and hourly clock functions.
 *   
 * See  https://github.com/pnjorge
 *      https://www.hackster.io/pnjorge
 *      paulonjorge@yahoo.com
 */


// This 4git version:  Customise unique SSID/Password and Network.h fileserver details.





#include "Arduino.h"
#include "WiFi.h"
#include "Audio.h"
#include "SPIFFS.h"
#include "Adafruit_MCP23017.h"
#include "FastLED.h"
#include "ArduinoJson.h"
#include "time.h"


#define SENSE_LED 2       // This is the LED_BUILTIN to the DevKit V1 board
#define sensePin      34  // GPIO34 (RTC_GPIO4)

#define I2S_DOUT      25  // DIN connection
#define I2S_LRC       26  // Left Right Clock
#define I2S_BCLK      27  // Bit clock



#define LED_PIN       33
#define NUM_LEDS      32
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB







const uint8_t mcp23017toStation[]   =    {5, 6, 7, 8, 9, 10, 12, 11, 15, 16, 14, 13, 4, 3, 2, 1};   // Station No.                
                  //  Library pin      :  0  1  2  3  4  5   6   7   8   9   10  11  12 13 14 15 
                  //  MCP23017 data pin:  A0 A1 A2 A3 A4 A5  A6  A7  B0  B1  B2  B3  B4 B5 B6 B7

uint8_t bright  =  0;     //Overall max led brightness level will be loaded from config file.
               

uint8_t volume = 0;        // range is 0 to 21


uint8_t   stationNow = 0;
uint8_t   stationPrevious = 0;
bool  mp3Playing = false;
bool  tuneNeeded = false;

bool  wifiGood = false;

char station01[75] = "";
char station02[75] = "";
char station03[75] = "";
char station04[75] = "";
char station05[75] = "";
char station06[75] = "";
char station07[75] = "";
char station08[75] = "";
char station09[75] = "";
char station10[75] = "";
char station11[75] = "";
char station12[75] = "";
char station13[75] = "";
char station14[75] = "";
char station15[75] = "";
char station16[75] = "";


unsigned long   zenNow          = 0;
int             zenToPlay       = 0;
int             zenStartAfter   = 30000;      //Start Zen/Clock Mode after 30secs
bool            zenClockMode    = false; 


unsigned long   zenRandomMin = 0;
unsigned long   zenRandomMax = 0;
unsigned long   zenRandomPeriod = 0;
unsigned long   zenRandomNow    = 0;

const int secPeriod = 1000;
unsigned long time_now = 0;


uint8_t hrNow      = 0;
uint8_t hrPrevious = 99;
uint8_t hourToPlay  = 0;

bool ntpTryOnce = true;
bool ntpIsUpdated = false;

Audio audio;
Adafruit_MCP23017 mcp;

CRGB leds[NUM_LEDS];


String ssid =     "Livebox-1234";               // Replace these with real SSID and PASSWORD 
String password = "abcde12345ABCDE";



const char* ntpServer = "pool.ntp.org";
                                  //  Will load correct values from config file
long  gmtOffset_sec = 0;          //  ex: Paris is UTC +1hour (3600secs)
int   daylightOffset_sec = 0;     //  ex: DST is +1hour (3600secs)



  uint8_t stationDialNow = 0;
  uint8_t stationDialPrevious = 0;
  
  bool noDialInput = true;

  bool configMode = false;
  bool playedOnceAlready = false;



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Globals for Fileserver:


#include <WiFiMulti.h>         // Built-in
#include <ESP32WebServer.h>    // https://github.com/Pedroalbuquerque/ESP32WebServer download and place in your Libraries folder
#include <ESPmDNS.h>


#include "Network.h"
#include "Sys_Variables.h"
#include "CSS.h"
#include <SPI.h>

#define ServerVersion "1.0"f

bool    SPIFFS_present = false;



  WiFiMulti wifiMulti;
  ESP32WebServer server(80);  


// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


void HomePage();              // function prototypes
void File_Download();
void File_Upload(); 
void handleFileUpload(); 
void File_Stream(); 
void File_Delete(); 
void SPIFFS_dir(); 






  
// ----------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------------------
// S E T U P
// ----------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------------------



void setup() {


  pinMode(sensePin, INPUT);

  pinMode (SENSE_LED, OUTPUT);

  Serial.begin(115200);
  delay(300);

  digitalWrite(SENSE_LED,HIGH);  

    
  Serial.println(); Serial.println(); Serial.println();
  
  Serial.print(F("VERSION: wifiradio_PJ_V85_4git"));     //*********************
                                                         //   VERSION NUMBER  
                                                         //*********************
  Serial.println(); Serial.println();
  Serial.print(F("Initializing SPIFFS... "));


  if (!SPIFFS.begin(true)) 
  { 
    Serial.println("SPIFFS initialisation failed.");
    SPIFFS_present = false; 
  }
  else
  {
    Serial.println(F("SPIFFS initialised... File access is enabled."));
    SPIFFS_present = true; 
  }
 


    
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    


    mcp.begin();            // Default device address is 00

        
        for (uint8_t mcpInput = 0; mcpInput <= 15; mcpInput++) 
        {
          
          mcp.pinMode(mcpInput,INPUT);  // Button i/p to GND
          mcp.pullUp(mcpInput,HIGH);    // Puled high to ~100k
     
        }
        Serial.println("MCP Inputs pulled high");
      

      fill_solid(leds, NUM_LEDS, CRGB(0,0,0) );  // Black
      FastLED.show();
      delay(300);


        if (!loadConfig()) {                           // Load config.json file
        Serial.println("Failed to load config");
        }
        else 
        {
        Serial.println("Config loaded");
        }


      Serial.print(F("volume loaded is             ")); Serial.println(volume);
      Serial.print(F("bright loaded is             ")); Serial.println(bright);
      Serial.print(F("zenRandomMin loaded is       ")); Serial.println(zenRandomMin);
      Serial.print(F("zenRandomMax loaded is       ")); Serial.println(zenRandomMax);
      Serial.print(F("gmtOffset_sec loaded is      ")); Serial.println(gmtOffset_sec);
      Serial.print(F("daylightOffset_sec loaded is ")); Serial.println(daylightOffset_sec);


      
      audio.setVolume(volume); 


    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    delay(500);
       
    FastLED.setBrightness(bright);                            // To set maximum brightness level. 
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 500);           // FastLED power management, to cap power usage if max bright too high. Ex: 5V, 500mA.



  stationNow = mcp23017toStation[ GetTuning() ]; 

//      stationNow = GetTuning(); 
//      stationNow = stationNow + 1;        //sensor on A0 is now station 1 etc...
      
      Serial.print("stationNow initially is   "); Serial.println(stationNow);
      
      Serial.print("configMode initially is   "); Serial.println(configMode);


      if ( noDialInput == true )
      {
        stationNow = 17;                  //station 17 is to be in config mode
        configMode = true;
        runFileServerSetup();
      }
      
      else
      
      {
        fill_solid(leds, NUM_LEDS, CRGB(0,255,0) );  // Normal Dial Mode leds, fill green.
        FastLED.show();
        delay(500);      
         
      }


}







// ----------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------------------
// L O O P
// ----------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------------------



void loop() {



      senseRadio();     // Check to see if radio is powered ON of OFF.


        //Serial.print(F("zenNow is ")); Serial.print(zenNow); Serial.print(F("        zenStartAfter is ")); Serial.print(zenStartAfter); Serial.print(F("        millis() is ")); Serial.println(millis()); Serial.print(F(" zenNow + zenStartAfter is ")); Serial.println(zenNow+zenStartAfter);

      
      audio.loop();


  if ( configMode == true)
  {
            

      if ( (mp3Playing == false) && (playedOnceAlready == false) )    //waits for mp3 announcement to finish 
      {
          Serial.print("configMode is now  "); Serial.println(configMode);
          announceStation();
          playedOnceAlready = true;
          
          fill_solid(leds, NUM_LEDS, CRGB(0,0,255) );  // Config Mode leds, fill blue.
          FastLED.show();
          delay(500);

          Serial.println();
          Serial.println("---------- IN CONFIG MODE ----------");
          
      }

      SPIFFSFileserver();       

  }




  else




  {

        
      if ( (wifiGood == false) && (mp3Playing == false) )   // Is wifi connected??
      {
        wifiConnect();                                      //Tries to connect to wifi and waits for 10secs
        if (wifiGood == false )
        {
          
          mp3Playing = true;
          audio.connecttoFS (SPIFFS, "wififail.mp3");

        }  

        
    }

    else

    {

      if ( wifiGood == true)
      {
        
/*     
      if ( mp3Playing == false )    //waits for mp3 announcement to finish 
      {
          wasStationChanged();
      }
*/


      if ( (tuneNeeded == true) && (mp3Playing == false) )
        {
          tuneStation();
          tuneNeeded = false;
        }
      
     
      wasStationChanged();

      
            //In Zen Clock mode:
                    
            if ( (zenClockMode == true) && (mp3Playing == false) )
            {

            rgbZen();
            

              if (  millis() >= (zenRandomNow + zenRandomPeriod)  )
                {            
                Serial.println("zenToPlay...playZen()");
              
                zenToPlay = random(1, 11);              //  1inclusive to 11exclusive is zen_01.mp3 to zen_10.mp3 respectively 

                zenRandomPeriod = random(zenRandomMin, zenRandomMax); //Next from 20secs to about 60secs. Period counting from this mp3 playing. 
                Serial.print("zenRandomPeriod  is  "); Serial.println(zenRandomPeriod);
              
                playZen();                              //  Play the random zen mp3 sound
                }
              
              else
             
                {
                  if (millis() >= time_now + secPeriod)     // checks every second
                  {
                    time_now += secPeriod;

       
      
                    hrNow = printLocalTime(); 

                    if ( (hrNow != hrPrevious) && ( ntpIsUpdated == true) )
                      {
                        Serial.println("Hours changed ********************");
                        hrPrevious = hrNow;

                        if ( hrNow >= 13 )
                        {
                          hrNow = hrNow - 12;        // Convert to 12hr clock                         
                        }
                        
                        hourToPlay = hrNow;
                        playHour(); 
                      }
            
                      
                  }
                
                }
              
            }

  
      }

    }  


  }


      
}










// ----------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------------------
// F U N C T I O N S 
// ----------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------------------------------------------------------------------------------------------




void rgbZen()
{

    static uint8_t h = 0;
    static int hDirection =  1;
          
          
    static unsigned long currentMillis = millis();
    
    if ( millis() >= currentMillis + 1000)
    {
     

      if (h >= 255) 
      {
        hDirection = -1;
      }

      if (h <= 0) 
      {
        hDirection = 1;
      }

      h = h + hDirection;
    
      fill_solid(leds, NUM_LEDS, CHSV(h,255,255));
      FastLED.show();

    currentMillis = millis();
    }


}







void senseRadio()
{

            //Serial.print("sensePin is "); Serial.println(digitalRead(sensePin));

            int radioTimeout = 0;

            while (digitalRead(sensePin) == LOW)      // No radio voltage detected on sense pin.
            { 
              digitalWrite(SENSE_LED,LOW);           // Flash builtin led.
              delay(250);  
              digitalWrite(SENSE_LED,HIGH); 
              delay(250);            
              Serial.print(".");
              radioTimeout++;

              
                if ( radioTimeout >= 20)                 // Waits 10secs (20 x 500mS)
                {
                  digitalWrite(SENSE_LED,LOW);  
                  
                  Serial.println(F("  Radio Timeout")); 
                  //Serial.print(F("WiFi status code = ")); Serial.println(WiFi.status());                 

                    for ( int fadeIdx = bright; fadeIdx >= 0; fadeIdx--)      // Fade ambient backlight
                    {
                      FastLED.setBrightness(fadeIdx);
                      delay(50);
                      FastLED.show();    
                    }
                 

                  
                  fill_solid(leds, NUM_LEDS, CRGB(0,0,0));      // let's make sure its really black
                  FastLED.show();
                  
                  radioTimeout = 0;
                  delay(1000);


                  Serial.println(F("Entering Deep Sleep Mode"));           
                  delay(1000);                  

                  esp_sleep_enable_ext0_wakeup(GPIO_NUM_34, 1);        // Set Sleep Mode ext0 wakeup
                  esp_deep_sleep_start();                             // Enter Deep Sleep 
                  
                }              

        
            }


}





void runFileServerSetup() 
{

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

  if (!WiFi.config(local_IP, gateway, subnet, dns)) { //WiFi.config(ip, gateway, subnet, dns1, dns2);
    Serial.println(F("WiFi STATION Failed to configure Correctly"));
  }
  
  wifiMulti.addAP(ssid_1, password_1);  // add Wi-Fi networks you want to connect to, it connects strongest to weakest
  wifiMulti.addAP(ssid_2, password_2);  // Adjust the values in the Network tab
  wifiMulti.addAP(ssid_3, password_3);
  wifiMulti.addAP(ssid_4, password_4);  // You don't need 4 entries, this is for example!

  Serial.println(F("Connecting ..."));
  while (wifiMulti.run() != WL_CONNECTED) { // Wait for the Wi-Fi to connect: scan for Wi-Fi networks, and connect to the strongest of the networks above
    delay(250); Serial.print(F('.'));
  }
  
  Serial.println("\nConnected to " + WiFi.SSID() + " Use IP address: " + WiFi.localIP().toString()); // Report which SSID and IP is in use
  // The logical name http://fileserver.local will also access the device if you have 'Bonjour' running or your system supports multicast dns
  if (!MDNS.begin(servername)) {          // Set your preferred server name, if you use "myserver" the address would be http://myserver.local/
    Serial.println(F("Error setting up MDNS responder!"));
    ESP.restart();
  }


  // Note: SD_Card readers on the ESP32 will NOT work unless there is a pull-up on MISO, either do this or wire one on (1K to 4K7)
  Serial.println(MISO);
  pinMode(19, INPUT_PULLUP);


  
  //----------------------------------------------------------------------   
  ///////////////////////////// Server Commands 
  server.on("/",         HomePage);
  server.on("/download", File_Download);
  server.on("/upload",   File_Upload);
  server.on("/fupload",  HTTP_POST,[](){ server.send(200);}, handleFileUpload);
  server.on("/stream",   File_Stream);
  server.on("/delete",   File_Delete);
  server.on("/dir",      SPIFFS_dir);
  
  ///////////////////////////// End of Request commands
  server.begin();
  Serial.println(F("HTTP server started"));



// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

}




// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~


bool loadConfig() {


  
  File configFile = SPIFFS.open("/config.json", "r");
  if (!configFile) {
    Serial.println(F("Failed to open config file"));
    return false;
  }


  size_t size = configFile.size();
  if (size > 3072) {
    Serial.println(F("Config file size is too large"));
    return false;
  }


  // Allocate a buffer to store contents of the file.
  std::unique_ptr<char[]> buf(new char[size]);


  // We don't use String here because ArduinoJson library requires the input
  // buffer to be mutable. If you don't use ArduinoJson, you may as well
  // use configFile.readString instead.
  configFile.readBytes(buf.get(), size);

  Serial.println(F("READING CONFIG FILE..."));
  Serial.println(buf.get());
  Serial.println();

  StaticJsonDocument<1024> doc;
  auto error = deserializeJson(doc, buf.get());
  if (error) {
    Serial.println(F("Failed to parse config file"));
    return false;
  }




  volume              = doc["volume"]; 
  bright              = doc["bright"]; 

  zenRandomMin        = doc["zenRandomMin"];
  zenRandomMin        = zenRandomMin*1000;         //convert to mSecs
     
  zenRandomMax        = doc["zenRandomMax"];
  zenRandomMax        = zenRandomMax*1000;         //convert to mSecs

  gmtOffset_sec       = doc["gmtOffset"];
  daylightOffset_sec  = doc["daylightOffset"];
  

  strcpy(station01, doc["radio_01"] );   
  strcpy(station02, doc["radio_02"] ); 
  strcpy(station03, doc["radio_03"] );   
  strcpy(station04, doc["radio_04"] ); 
  strcpy(station05, doc["radio_05"] );   
  strcpy(station06, doc["radio_06"] ); 
  strcpy(station07, doc["radio_07"] );   
  strcpy(station08, doc["radio_08"] ); 
  strcpy(station09, doc["radio_09"] );   
  strcpy(station10, doc["radio_10"] ); 
  strcpy(station11, doc["radio_11"] );   
  strcpy(station12, doc["radio_12"] ); 
  strcpy(station13, doc["radio_13"] );   
  strcpy(station14, doc["radio_14"] ); 
  strcpy(station15, doc["radio_15"] );   
  strcpy(station16, doc["radio_16"] ); 


 
  return true;
  
}

// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~




 

void wifiConnect()
{

    
    WiFi.disconnect();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    delay(3000);                                  //wait a bit to stabilize

    Serial.println();
    Serial.print(F("Connecting to WiFi: "));   Serial.println(ssid);
    
    int waitWifi = 0;

    wifiGood = true;

    
      while (WiFi.status() != WL_CONNECTED) 
      {
      
      delay(250); Serial.print(F("."));
      waitWifi++;
      
        if ( waitWifi >= 40)                 // Waits 10secs to connect to WiFi (40 x 250mSecs)
        {
          Serial.println();
          Serial.println(F("!! Failed to connect to WiFi !!"));
          wifiGood = false;
          
          break;
        }               
      }
  
   Serial.println(); 
  
}




// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// LED Functions


void ledTuneDial() {

  int ledsToLight = ( stationNow * 2);

  fill_solid(leds, NUM_LEDS, CRGB(0,255,0));  // test leds, fill green.
  FastLED.show();

  
  if ( stationNow > 0)
  { 

  leds[ledsToLight - 2] = CRGB(255,0,0);  leds[ledsToLight - 1] = CRGB(255,0,0);
  FastLED.show();

  }


  
}



void tuneFlicker() { 
  
  int ledsToLight = ( stationNow * 2);


  fill_solid(leds, NUM_LEDS, CRGB(0,255,0) );  
  FastLED.show();

  if ( stationNow > 0)
  {  
    
    for ( int i = 0; i <= 25 ; i++)                     //Tuning Lock flicker
    {

      leds[ledsToLight - 2] = CRGB(255,0,0);  leds[ledsToLight - 1] = CRGB(255,0,0);
      FastLED.show();
      delay(i*2);
      
      leds[ledsToLight - 2] = CRGB(0,0,0);  leds[ledsToLight - 1] = CRGB(0,0,0);
      FastLED.show();
      delay(i*2);
    }
  
    leds[ledsToLight - 2] = CRGB(255,0,0);  leds[ledsToLight - 1] = CRGB(255,0,0);
    FastLED.show();
  
  }
  

}





// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Radio Functions



void wasStationChanged()
{


  stationNow = mcp23017toStation[ GetTuning() ];    
     
      //Serial.print(F("zenNow is ")); Serial.print(zenNow); Serial.print(F("        zenStartAfter is ")); Serial.print(zenStartAfter); Serial.print(F("        millis() is ")); Serial.println(millis()); Serial.print(F(" zenNow + zenStartAfter is ")); Serial.println(zenNow+zenStartAfter);

  if (noDialInput == true) 
  {
    stationNow = 0;           //between stations, so will play hfsqueal.mp3 

      //Serial.print(F("zenNow is ")); Serial.print(zenNow); Serial.print(F("        zenStartAfter is ")); Serial.print(zenStartAfter); Serial.print(F("        millis() is ")); Serial.println(millis()); Serial.print(F(" zenNow + zenStartAfter is ")); Serial.println(zenNow+zenStartAfter);

      if ( (millis() >=  (zenNow + zenStartAfter)) && (zenClockMode == true) && (ntpTryOnce == true)  )
      {

            configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);       //init and configure for time from NTP server
            Serial.println("*** Time updated from NTP server ***");
            
              mp3Playing = true;
              audio.connecttoFS (SPIFFS, "ntptime.mp3");

            hrPrevious = printLocalTime();
                                                                     
            ntpIsUpdated = true;
            ntpTryOnce = false;
            
                  
      }

  }



  if ( stationNow != stationPrevious )
  {  
    Serial.println(F("stationNow != stationPrevious"));
    Serial.print(F("stationPrevious: "));   Serial.println(stationPrevious); 
    Serial.print(F("stationNow: "));   Serial.println(stationNow); 
    announceStation();
    stationPrevious = stationNow;  
  }
  
}








uint8_t printLocalTime()
{


//Serial.println("print time here");
  uint8_t hourTime;

  struct tm timeinfo;                                     //http://www.cplusplus.com/reference/ctime/tm/


  
  if( !getLocalTime(&timeinfo) )  
  {
    Serial.println("Failed to obtain time!");
   
    return 99;
  }


  
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");     //http://www.cplusplus.com/reference/ctime/strftime/


  //hourTime = timeinfo.tm_min;  
  //...for testing
  
  hourTime = timeinfo.tm_hour;
  
  return hourTime;
  
}








uint8_t GetTuning()
{

  //Serial.println(F("***** Entered GetTuning()")); 
  
  int dialInput = 1;
  
  noDialInput = true;
  
 for (uint8_t mcpInput = 0; mcpInput <= 15; mcpInput++)   
  {
  dialInput = mcp.digitalRead(mcpInput);
/*
  Serial.print(F("scanning mcpInput No. ")); Serial.print(mcpInput);
  Serial.print(F("  =  ")); Serial.print(mcp.digitalRead(mcpInput));
  Serial.print(F(" ..dialInput is: ")); Serial.println(dialInput);
*/
    audio.loop();    //Need this call here because this for loop slows things down and causes breaks in audio
     

     if (dialInput == 0) 
     {
      noDialInput = false;
      stationDialNow = mcpInput;

         if (stationDialNow != stationDialPrevious) 
         {
          
           Serial.println(F("stationDialNow !!!!!!= stationDialPrevious")); 
           Serial.print(F("stationDialPrevious was  ")); Serial.println(stationDialPrevious);
           Serial.print(F("stationDialNow is  ")); Serial.println(stationDialNow);
           stationDialPrevious = stationDialNow;

           break;       
         }      
     }



       
  }


  return uint8_t( (stationDialNow) );   

}







void tuneStation()
{


    switch (stationNow)                      
  {

    case 0:   

    Serial.println(F("case 0 ___________________ Dummy Tune _________________"));
          
    break;
    
    
    case 1:   

    Serial.println(F("....Tuning to Station 1"));      
    audio.connecttohost(station01);     
    break;


    case 2:   
    Serial.println(F("....Tuning to Station 2"));
    audio.connecttohost(station02);      
    break;


    case 3:   
    Serial.println(F("....Tuning to Station 3")); 
    audio.connecttohost(station03);          
    break;


    case 4:   
    Serial.println(F("....Tuning to Station 4"));   
    audio.connecttohost(station04);          
    break;


    case 5:   
    Serial.println(F("....Tuning to Station 5"));    
    audio.connecttohost(station05);          
    break;


    case 6:   
    Serial.println(F("....Tuning to Station 6"));   
    audio.connecttohost(station06);          
    break;
    

    case 7:   
    Serial.println(F("....Tuning to Station 7"));   
    audio.connecttohost(station07);          
    break;


    case 8:   
    Serial.println(F("....Tuning to Station 8"));
    audio.connecttohost(station08);          
    break;


    case 9:   
    Serial.println(F("....Tuning to Station 9"));
    audio.connecttohost(station09);          
    break;    


    case 10:   
    Serial.println(F("....Tuning to Station 10"));  
    audio.connecttohost(station10);          
    break;


    case 11:   
    Serial.println(F("....Tuning to Station 11"));  
    audio.connecttohost(station11);          
    break;    

    
    case 12:   
    Serial.println(F("....Tuning to Station 12"));   
    audio.connecttohost(station12);          
    break;


    case 13:   
    Serial.println(F("....Tuning to Station 13"));    
    audio.connecttohost(station13);          
    break;



    case 14:   
    Serial.println(F("....Tuning to Station 14"));  
    audio.connecttohost(station14);          
    break;



    case 15:   
    Serial.println(F("....Tuning to Station 15")); 
    audio.connecttohost(station15);          
    break;



    case 16:   
    Serial.println(F("....Tuning to Station 16"));
    audio.connecttohost(station16);          
    break;


    
    default:    
     // Not here.    
    break;
    
  }

  tuneFlicker();
  
}





void announceStation()
{


    tuneNeeded = true;
 
    ledTuneDial();


  
    switch (stationNow)                      
  {


    case 0:   
    Serial.println(F("Case 0__Between stations"));
              
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "hfsqueal.mp3");         // Simulate radio tuning sound
            
            zenNow = millis(); 
            zenClockMode = true;                    //sets zenClockMode flag
            zenRandomNow = millis();
            zenRandomPeriod = 45000;                // Waits 45secs before first zen mp3 play, after going in to Zen Clock Mode 
                                                    //... and allows for NTP error mp3 announcement.  
    break;          


    
    case 1:   
    Serial.println(F("Announce 1:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "radio_01.mp3");
            zenClockMode = false;      
    break;


    case 2:   
    Serial.println(F("Announce 2:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "radio_02.mp3");
            zenClockMode = false;
    break;

    
    case 3:   
    Serial.println(F("Announce 3:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "radio_03.mp3");
            zenClockMode = false;
    break;

    case 4:   
    Serial.println(F("Announce 4:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "radio_04.mp3");
            zenClockMode = false;
    break;


    case 5:   
    Serial.println(F("Announce 5:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "radio_05.mp3");
            zenClockMode = false;
    break;    

    case 6:   
    Serial.println(F("Announce 6:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "radio_06.mp3");
            zenClockMode = false;
    break;   


    case 7:   
    Serial.println(F("Announce 7:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "radio_07.mp3");
            zenClockMode = false;
    break;   

    
    case 8:   
    Serial.println(F("Announce 8:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "radio_08.mp3");
            zenClockMode = false;
    break;   


    case 9:   
    Serial.println(F("Announce 9:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "radio_09.mp3");
            zenClockMode = false;
    break;   


    case 10:   
    Serial.println(F("Announce 10:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "radio_10.mp3");
            zenClockMode = false;
    break;   


    case 11:   
    Serial.println(F("Announce 11:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "radio_11.mp3");
            zenClockMode = false;
    break;   



    case 12:   
    Serial.println(F("Announce 12:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "radio_12.mp3");
            zenClockMode = false;
    break;   



    case 13:   
    Serial.println(F("Announce 13:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "radio_13.mp3");
            zenClockMode = false;
    break;   


    case 14:   
    Serial.println(F("Announce 14:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "radio_14.mp3");
            zenClockMode = false;
    break;   


    case 15:   
    Serial.println(F("Announce 15:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "radio_15.mp3");
            zenClockMode = false;
    break;   



    case 16:   
    Serial.println(F("Announce 16:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "radio_16.mp3");
            zenClockMode = false;
    break;   


    case 17:   
    Serial.println(F("Announce 17/config mode:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "inconfig.mp3");           // 17 is inconfig.mp3 Configuration Mode announcement           

    break;   

    
    default:    
     // Not here.    
    break;

  }

    
}






void playHour()
{


      mp3Playing = true;
      audio.connecttoFS (SPIFFS, "chime.mp3");    
    
      while ( mp3Playing == true )                // Waits for chime.mp3 to finish playing
      {
        audio.loop();
      }
               


    
  
  switch (hourToPlay)                      
  {


    case 0:   
    Serial.println(F("Announce midnight")); 
    
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "00oclock.mp3");         
            
    break;        




    case 1:   
    Serial.println(F("Announce 1 oclock")); 
    
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "01oclock.mp3");         
            
    break;        



    case 2:   
    Serial.println(F("Announce 2 oclock")); 
    
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "02oclock.mp3");         
            
    break;    


    case 3:   
    Serial.println(F("Announce 3 oclock")); 
    
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "03oclock.mp3");         
            
    break;    




    case 4:   
    Serial.println(F("Announce 4 oclock")); 
    
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "04oclock.mp3");         
            
    break;        



    case 5:   
    Serial.println(F("Announce 5 oclock")); 
    
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "05oclock.mp3");         
            
    break;    


    case 6:   
    Serial.println(F("Announce 6 oclock")); 
    
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "06oclock.mp3");         
            
    break;    



    case 7:   
    Serial.println(F("Announce 7 oclock")); 
    
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "07oclock.mp3");         
            
    break;        



    case 8:   
    Serial.println(F("Announce 8 oclock")); 
    
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "08oclock.mp3");         
            
    break;    


    case 9:   
    Serial.println(F("Announce 9 oclock")); 
    
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "09oclock.mp3");         
            
    break;    




    case 10:   
    Serial.println(F("Announce 10 oclock")); 
    
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "10oclock.mp3");         
            
    break;        



    case 11:   
    Serial.println(F("Announce 11 oclock")); 
    
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "11oclock.mp3");         
            
    break;    


    case 12:   
    Serial.println(F("Announce 12 oclock")); 
    
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "12oclock.mp3");         
            
    break;  


    default:    
     // Not here.    
    break;

/*    
    default:    // and for TESTING above 23 mins with tm_min
    Serial.println(F("Announce time set")); 
    
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "ntptime.mp3");  
      
    break;    
*/
  
  }


}






void playZen()
{


  zenRandomNow = millis();
  
  switch (zenToPlay)                      
  {


    case 1:   
    Serial.println(F("zen01:"));
    
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "zen_01.mp3");           // zen_01.mp3 is bird singing 1           

    break;   



    case 2:   
    Serial.println(F("zen02:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "zen_02.mp3");           // zen_02.mp3 is bird singing 2           

    break;   



    case 3:   
    Serial.println(F("zen03:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "zen_03.mp3");           // zen_03.mp3 is bird singing 3           

    break;   



    case 4:   
    Serial.println(F("zen04:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "zen_04.mp3");           // zen_04.mp3 is bird singing 4           

    break;   


    case 5:   
    Serial.println(F("zen05:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "zen_05.mp3");           // zen_05.mp3 is bird singing 5           

    break;   

     

    case 6:   
    Serial.println(F("zen06:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "zen_06.mp3");           // zen_06.mp3 is bird singing 6           

    break;   


      
    case 7:   
    Serial.println(F("zen07:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "zen_07.mp3");           // zen_07.mp3 is bird singing 7           

    break;   
   


    case 8:   
    Serial.println(F("zen08:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "zen_08.mp3");           // zen_08.mp3 is bird singing 8           

    break;   


      

    case 9:   
    Serial.println(F("zen09:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "zen_09.mp3");           // zen_09.mp3 is bird singing 9           

    break;   


      
    case 10:   
    Serial.println(F("zen10:"));
            mp3Playing = true;
            audio.connecttoFS (SPIFFS, "zen_10.mp3");           // zen_10.mp3 is bird singing 10           

    break;   



    
    default:    
     // Not here.    
    break;
      
  }

  
}







// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Optional Functions (PCM5102 feedback)


// optional

void audio_eof_mp3(const char *info){  //end of file
    Serial.print("eof_mp3     ");Serial.println(info);
    mp3Playing = false;
    Serial.print("mp3Playing  ");Serial.println(mp3Playing);    
}

void audio_info(const char *info){
    Serial.print("info        "); Serial.println(info);
}

void audio_id3data(const char *info){  //id3 metadata
    Serial.print("id3data     ");Serial.println(info);
}

void audio_showstation(const char *info){
    Serial.print("station     ");Serial.println(info);
}

void audio_showstreaminfo(const char *info){
    Serial.print("streaminfo  ");Serial.println(info);
}

void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle ");Serial.println(info);
}

void audio_bitrate(const char *info){
    Serial.print("bitrate     ");Serial.println(info);
}
void audio_commercial(const char *info){  //duration in sec
    Serial.print("commercial  ");Serial.println(info);
}

void audio_icyurl(const char *info){  //homepage
    Serial.print("icyurl      ");Serial.println(info);
}
void audio_lasthost(const char *info){  //stream URL played
    Serial.print("lasthost    ");Serial.println(info);
}

void audio_eof_speech(const char *info){
    Serial.print("eof_speech  ");Serial.println(info);    
}







// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Functions for Fileserver:    
// ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



void SPIFFSFileserver(void)
{
  server.handleClient(); // Listen for client connections
}




// All supporting functions from here...

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void HomePage(){
  SendHTML_Header();
  webpage += F("<a href='/download'><button>Download</button></a>");
  webpage += F("<a href='/upload'><button>Upload</button></a>");
  webpage += F("<a href='/stream'><button>Stream</button></a>");
  webpage += F("<a href='/delete'><button>Delete</button></a>");
  webpage += F("<a href='/dir'><button>Directory</button></a>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop(); // Stop is needed because no content length was sent
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Download(){ // This gets called twice, the first pass selects the input, the second pass then processes the command line arguments
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("download")) DownloadFile(server.arg(0));
  }
  else SelectInput("Enter filename to download","download","download");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void DownloadFile(String filename){
  if (SPIFFS_present) { 
    File download = SPIFFS.open("/"+filename,  "r");
    if (download) {
      server.sendHeader("Content-Type", "text/text");
      server.sendHeader("Content-Disposition", "attachment; filename="+filename);
      server.sendHeader("Connection", "close");
      server.streamFile(download, "application/octet-stream");
      download.close();
    } else ReportFileNotPresent("download"); 
  } else ReportSPIFFSNotPresent();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Upload(){
  append_page_header();
  webpage += F("<h3>Select File to Upload</h3>"); 
  webpage += F("<FORM action='/fupload' method='post' enctype='multipart/form-data'>");
  webpage += F("<input class='buttons' style='width:40%' type='file' name='fupload' id = 'fupload' value=''><br>");
  webpage += F("<br><button class='buttons' style='width:10%' type='submit'>Upload File</button><br>");
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  server.send(200, "text/html",webpage);
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
File UploadFile; 
void handleFileUpload(){ // upload a new file to the Filing system
  HTTPUpload& uploadfile = server.upload(); // See https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/srcv
                                            // For further information on 'status' structure, there are other reasons such as a failed transfer that could be used
  if(uploadfile.status == UPLOAD_FILE_START)
  {
    String filename = uploadfile.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    Serial.print("Upload File Name: "); Serial.println(filename);
    SPIFFS.remove(filename);                  // Remove a previous version, otherwise data is appended the file again
    UploadFile = SPIFFS.open(filename, "w");  // Open the file for writing in SPIFFS (create it, if doesn't exist)
  }
  else if (uploadfile.status == UPLOAD_FILE_WRITE)
  {
    if(UploadFile) UploadFile.write(uploadfile.buf, uploadfile.currentSize); // Write the received bytes to the file
  } 
  else if (uploadfile.status == UPLOAD_FILE_END)
  {
    if(UploadFile)          // If the file was successfully created
    {                                    
      UploadFile.close();   // Close the file again
      Serial.print("Upload Size: "); Serial.println(uploadfile.totalSize);
      webpage = "";
      append_page_header();
      webpage += F("<h3>File was successfully uploaded</h3>"); 
      webpage += F("<h2>Uploaded File Name: "); webpage += uploadfile.filename+"</h2>";
      webpage += F("<h2>File Size: "); webpage += file_size(uploadfile.totalSize) + "</h2><br>"; 
      append_page_footer();
      server.send(200,"text/html",webpage);
    } 
    else
    {
      ReportCouldNotCreateFile("upload");
    }
  }
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#ifdef ESP32
void SPIFFS_dir(){ 
  if (SPIFFS_present) { 
    File root = SPIFFS.open("/");
    if (root) {
      root.rewindDirectory();
      SendHTML_Header();
      webpage += F("<h3 class='rcorners_m'>Contents</h3><br>");
      webpage += F("<table align='center'>");
      webpage += F("<tr><th>Name/Type</th><th style='width:20%'>Type File/Dir</th><th>File Size</th></tr>");
      printDirectory("/",0);
      webpage += F("</table>");
      SendHTML_Content();
      root.close();
    }
    else 
    {
      SendHTML_Header();
      webpage += F("<h3>No Files Found</h3>");
    }
    append_page_footer();
    SendHTML_Content();
    SendHTML_Stop();   // Stop is needed because no content length was sent
  } else ReportSPIFFSNotPresent();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void printDirectory(const char * dirname, uint8_t levels){
  File root = SPIFFS.open(dirname);
  if(!root){
    return;
  }
  if(!root.isDirectory()){
    return;
  }
  File file = root.openNextFile();
  while(file){
    if (webpage.length() > 1000) {
      SendHTML_Content();
    }
    if(file.isDirectory()){
      webpage += "<tr><td>"+String(file.isDirectory()?"Dir":"File")+"</td><td>"+String(file.name())+"</td><td></td></tr>";
      printDirectory(file.name(), levels-1);
    }
    else
    {
      webpage += "<tr><td>"+String(file.name())+"</td>";
      webpage += "<td>"+String(file.isDirectory()?"Dir":"File")+"</td>";
      int bytes = file.size();
      String fsize = "";
      if (bytes < 1024)                     fsize = String(bytes)+" B";
      else if(bytes < (1024 * 1024))        fsize = String(bytes/1024.0,3)+" KB";
      else if(bytes < (1024 * 1024 * 1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
      else                                  fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
      webpage += "<td>"+fsize+"</td></tr>";
    }
    file = root.openNextFile();
  }
  file.close();
}
#endif
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Stream(){
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("stream")) SPIFFS_file_stream(server.arg(0));
  }
  else SelectInput("Enter a File to Stream","stream","stream");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SPIFFS_file_stream(String filename) { 
  if (SPIFFS_present) { 
    File dataFile = SPIFFS.open("/"+filename,  "r"); // Now read data from SPIFFS Card 
    if (dataFile) { 
      if (dataFile.available()) { // If data is available and present 
        String dataType = "application/octet-stream"; 
        if (server.streamFile(dataFile, dataType) != dataFile.size()) {Serial.print(F("Sent less data than expected!")); } 
      }
      dataFile.close(); // close the file: 
    } else ReportFileNotPresent("Cstream");
  } else ReportSPIFFSNotPresent(); 
}   
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void File_Delete(){
  if (server.args() > 0 ) { // Arguments were received
    if (server.hasArg("delete")) SPIFFS_file_delete(server.arg(0));
  }
  else SelectInput("Select a File to Delete","delete","delete");
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SPIFFS_file_delete(String filename) { // Delete the file 
  if (SPIFFS_present) { 
    SendHTML_Header();
    File dataFile = SPIFFS.open("/"+filename, "r"); // Now read data from SPIFFS Card 
    if (dataFile)
    {
      if (SPIFFS.remove("/"+filename)) {
        Serial.println(F("File deleted successfully"));
        webpage += "<h3>File '"+filename+"' has been erased</h3>"; 
        webpage += F("<a href='/delete'>[Back]</a><br><br>");
      }
      else
      { 
        webpage += F("<h3>File was not deleted - error</h3>");
        webpage += F("<a href='delete'>[Back]</a><br><br>");
      }
    } else ReportFileNotPresent("delete");
    append_page_footer(); 
    SendHTML_Content();
    SendHTML_Stop();
  } else ReportSPIFFSNotPresent();
} 
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Header(){
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate"); 
  server.sendHeader("Pragma", "no-cache"); 
  server.sendHeader("Expires", "-1"); 
  server.setContentLength(CONTENT_LENGTH_UNKNOWN); 
  server.send(200, "text/html", ""); // Empty content inhibits Content-length header so we have to close the socket ourselves. 
  append_page_header();
  server.sendContent(webpage);
  webpage = "";
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Content(){
  server.sendContent(webpage);
  webpage = "";
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SendHTML_Stop(){
  server.sendContent("");
  server.client().stop(); // Stop is needed because no content length was sent
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void SelectInput(String heading1, String command, String arg_calling_name){
  SendHTML_Header();
  webpage += F("<h3>"); webpage += heading1 + "</h3>"; 
  webpage += F("<FORM action='/"); webpage += command + "' method='post'>"; // Must match the calling argument e.g. '/chart' calls '/chart' after selection but with arguments!
  webpage += F("<input type='text' name='"); webpage += arg_calling_name; webpage += F("' value=''><br>");
  webpage += F("<type='submit' name='"); webpage += arg_calling_name; webpage += F("' value=''><br><br>");
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReportSPIFFSNotPresent(){
  SendHTML_Header();
  webpage += F("<h3>No SPIFFS Card present</h3>"); 
  webpage += F("<a href='/'>[Back]</a><br><br>");
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReportFileNotPresent(String target){
  SendHTML_Header();
  webpage += F("<h3>File does not exist</h3>"); 
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void ReportCouldNotCreateFile(String target){
  SendHTML_Header();
  webpage += F("<h3>Could Not Create Uploaded File (write-protected?)</h3>"); 
  webpage += F("<a href='/"); webpage += target + "'>[Back]</a><br><br>";
  append_page_footer();
  SendHTML_Content();
  SendHTML_Stop();
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
String file_size(int bytes){
  String fsize = "";
  if (bytes < 1024)                 fsize = String(bytes)+" B";
  else if(bytes < (1024*1024))      fsize = String(bytes/1024.0,3)+" KB";
  else if(bytes < (1024*1024*1024)) fsize = String(bytes/1024.0/1024.0,3)+" MB";
  else                              fsize = String(bytes/1024.0/1024.0/1024.0,3)+" GB";
  return fsize;
}

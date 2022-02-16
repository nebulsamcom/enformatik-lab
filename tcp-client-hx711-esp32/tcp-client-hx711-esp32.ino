#include <WiFi.h>
#include <HX711_ADC.h>
#include <ArduinoJson.h>
#if defined(ESP8266)|| defined(ESP32) || defined(AVR)
#include <EEPROM.h>
#endif
void create_json(float weight);

const char* ssid      = "AndroidAPC041";
const char* password  = "gbli4059";
String JSON;

const uint16_t port = 9898; // port TCP server
const char * host = "192.168.56.214"; // ip or dns

// 1. HX711 circuit wiring
//pins:
const int HX711_dout = 2; //mcu > HX711 dout pin
const int HX711_sck = 4; //mcu > HX711 sck pin


//HX711 constructor:
HX711_ADC LoadCell(HX711_dout, HX711_sck);
WiFiClient client;

const int calVal_eepromAdress = 0;
unsigned long t = 0;

  
void setup() 
{
  
    Serial.begin(57600);
    WiFi.begin(ssid, password);
    

    Serial.println("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) 
    {  
      Serial.print(".");
      delay(1000);
    }
    
    Serial.println("WiFi connected\nIP address: ");Serial.println(WiFi.localIP());
    Serial.println("Connecting to ");Serial.println(host); 
    
    if (!client.connect(host, port)) 
    {
        Serial.println("Connection failed.");
        delay(2500);return;
    }

    //////////////HX711/////////////////////////////////////////

    
    LoadCell.begin();
    //LoadCell.setReverseOutput(); //uncomment to turn a negative output value to positive
    float calibrationValue; // calibration value (see example file "Calibration.ino")
    calibrationValue = 696.0; // uncomment this if you want to set the calibration value in the sketch
 
  #if defined(ESP8266)|| defined(ESP32)
    EEPROM.begin(512); // uncomment this if you use ESP8266/ESP32 and want to fetch the calibration value from eeprom
  #endif
  //EEPROM.get(calVal_eepromAdress, calibrationValue); // uncomment this if you want to fetch the calibration value from eeprom
  
    unsigned long stabilizingtime = 2000; // preciscion right after power-up can be improved by adding a few seconds of stabilizing time
    boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
  
    LoadCell.start(stabilizingtime, _tare);
    
    if (LoadCell.getTareTimeoutFlag()) 
    {
      Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
      while (1);
    }
    else 
    {
      LoadCell.setCalFactor(calibrationValue); // set calibration value (float)
      Serial.println("Startup is complete of hx711");
    }
}
  
void loop()
{
      int maxloops = 0;
      float i ;
      static boolean newDataReady = 0;
      const int serialPrintInterval = 0; //increase value to slow down serial print activity

  
      if (LoadCell.update()) newDataReady = true;

      // get smoothed value from the dataset:
      if (newDataReady) 
      {
        if (millis() > t + serialPrintInterval) 
        {
          i = LoadCell.getData();
          Serial.print("Load_cell output val: ");
          Serial.println(i*-10);
          newDataReady = 0;
          t = millis();
        }
      }
    
      create_json(i*-10);

      client.print(JSON);
      
  //    wait for the server's reply to become available
      while (!client.available() && maxloops < 1000)
      {
        maxloops++;
        delay(1); //delay 1 msec
      }
 
//    delay(100);
}

void create_json(float weight)
{
    StaticJsonDocument<500> doc;
  
    JsonObject weight_scaler = doc.to<JsonObject>();
    
    weight_scaler["weight_scaler"] = weight;
    
  
    serializeJsonPretty(doc, Serial);
    serializeJsonPretty(doc, JSON);

}

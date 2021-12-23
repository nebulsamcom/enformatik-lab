#include <WiFi.h>
#include <HX711.h>
#include <ArduinoJson.h>

void create_json(float weight);

const char* ssid      = "karun";
const char* password  = "12345678";
String json;

const uint16_t port = 52275; // port TCP server
const char * host = "192.168.43.130"; // ip or dns

// 1. HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 2;
const int LOADCELL_SCK_PIN = 4;

// 2. Adjustment settings
const long LOADCELL_OFFSET = 50682624;
const long LOADCELL_DIVIDER = 5895655;
  

WiFiClient client;
HX711 loadcell;
  
void setup() 
{
  
    Serial.begin(57600);
    WiFi.begin(ssid, password);
    
    loadcell.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
    loadcell.set_scale(LOADCELL_DIVIDER);
    loadcell.set_offset(LOADCELL_OFFSET);

    Serial.println("Connecting to WiFi");
      
    while (WiFi.status() != WL_CONNECTED) 
    {  
      Serial.print(".");
      delay(500);
    }
    
    Serial.println("WiFi connected\nIP address: ");Serial.println(WiFi.localIP());
    
    Serial.println("Connecting to ");Serial.println(host); 
    
    if (!client.connect(host, port)) 
    {
        Serial.println("Connection failed.");
        delay(5000);
        return;
    }
}
  
void loop()
{
    int maxloops = 0;
    create_json(loadcell.get_units(10));
    Serial.print("Weight: ");Serial.println(loadcell.get_units(10), 2);
    Serial.print("Weight json: ");Serial.println(json);
    
//    client.print(loadcell.get_units(10));
    client.print(json);
    
//    wait for the server's reply to become available
    while (!client.available() && maxloops < 1000)
    {
      maxloops++;
      delay(1); //delay 1 msec
    }
//    
//    delay(100);
}

void create_json(float weight)
{
  StaticJsonDocument<500> doc;

  JsonObject weight_scaler = doc.to<JsonObject>();
  
  weight_scaler["weight_scaler"] = weight;
  

  serializeJsonPretty(doc, Serial);
  serializeJsonPretty(doc, json);

}

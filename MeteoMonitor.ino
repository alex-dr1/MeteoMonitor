#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h" // legacy include: `#include "SSD1306.h"`

#include "fMon2038.h"

#include <ArduinoJson.h>
#include "unixTimeConv.h"

#define SCREEN_DURATION 10000
#define REQ_GETMETEO_DURATION 60
#define TIME_ZONE_UTC 5

SSD1306Wire display(0x3c, D2, D1);

typedef void (*Screen)(void);

int screenNo = 0;
int counter = 1;

int t_trend = 0;
int p_trend = 0;
double p1_old, t1_old;
bool start = true;

String statusLine;
String meteoDataTime, meteoTemp, meteoAtmPress;
String meteoDataTime_old = "";

const char* ssid = "Router81";
const char* password = "12341234";

void getMeteoData()
{
  //Serial.println("Connecting WiFi");
  WiFi.begin(ssid, password);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED) 
    {
      delay(1000);
      i++;
      if(i>12) break;
     }

  if (WiFi.status() == WL_CONNECTED) //Check WiFi connection status
  { 
    
  
    HTTPClient http;  //Declare an object of class HTTPClient
    
    //unsigned char* hash=MD5::make_hatrendsh("Esp8266Monitor");
    //char *md5str = MD5::make_digest(hash, 16);
    //free(hash);

     
   //Пример запроса JSON(POST):
   // {"cmd":"sensorsValues","sensors":[1,2],"uuid":"UUID","api_key":"API_KEY"}
   
    String request = "{\"cmd\":\"sensorsValues\",\"sensors\":[5598,14794],\"uuid\":\"66aeb0aaa005c2bb6e764b95790f7223\",\"api_key\":\"h6ZfZ2S2rQE7Y\"}"; 
    http.begin("http://narodmon.ru/api");  //Specify request destination
    int httpCode = http.POST(request);     //Send the request
     
    if (httpCode > 0) //Check the returning code
    { 
    	statusLine = "";//"Successfully received";
    	String json = http.getString();
      	// Deserialize the JSON document
      StaticJsonDocument<600> doc;
 	 	  DeserializationError error = deserializeJson(doc, json);

  		// Test if parsing succeeds.
  		if (error) 
  		{
  			statusLine = "deserializeJson - failed ";
  			//meteoTemp = "------";
  			//meteoAtmPress = "------";
  		}

      //Serial.println(json);
  		meteoDataTime = unixTimeConv(doc["sensors"][0]["time"], TIME_ZONE_UTC);
     
     
      meteoDataTime_old = meteoDataTime;
      meteoDataTime = /*meteoDataTime.substring(9,19)+"  "+*/meteoDataTime.substring(0,5);
      
      double t1 = doc["sensors"][0]["value"];
  		meteoTemp = String(t1, 1);

      double p1 = doc["sensors"][1]["value"];
      meteoAtmPress = String(p1, 1);


      if(!start){
         if(meteoDataTime_old == meteoDataTime)
         {
            statusLine = "Outdated data";
          }
          else
          {
            if(t1>=t1_old)
            {
              if(t_trend<0){
                t_trend = 0;
              }
              else
              {
                if(t_trend>=0&&t_trend<5)t_trend++;
              }
            }
    
            if(t1<=t1_old)
            {
              if(t_trend>0){
                t_trend = 0;
              }
              else
              {
                if(t_trend>-5&&t_trend<=0)t_trend--;
              }
            }
    
            if(p1>=p1_old)
            {
              if(p_trend<0){
                p_trend = 0;
              }
              else
              {
                if(p_trend>=0&&p_trend<5)p_trend++;
              }
            }
    
            if(p1<=p1_old)
            {
              if(p_trend>0)
              {
              p_trend = 0;
              }
              else
              {
                if(p_trend>-5&&p_trend<=0)p_trend--;
              }
            }
      
  
          t1_old = t1;
          p1_old = p1; 
          } 
      }     
      start = false;  
    }
    else
    {
      statusLine = "Error on POST request";
    } 
    http.end();   //Close connection
   
  }
  else
  {
    //Serial.println("Error in WiFi connection");
    statusLine = "Error in WiFi connection";
  }
  WiFi.disconnect();
}



void setup() {
  
  Serial.begin(9600);
  Serial.println();
  Serial.println();


  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT); 
  display.setFont(ArialMT_Plain_16);
  display.drawString(10, 16, "Starting");
  display.drawString(10, 32, "data retrieval...");
  display.display();

  getMeteoData();
}

void TemperaturaScreen() {
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(128, 0, meteoDataTime);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(fMon2038);
  display.drawString(128, 16, meteoTemp+"/");

  display.setTextAlignment(TEXT_ALIGN_LEFT); 
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "t"+String(t_trend));
  
  if(t_trend==-5){
      display.setTextAlignment(TEXT_ALIGN_LEFT); 
      display.setFont(fMon2038);
      display.drawString(0, 16, ">");  
  }
  
  if(t_trend>-5&&t_trend<=-2){
      display.setTextAlignment(TEXT_ALIGN_LEFT); 
      display.setFont(fMon2038);
      display.drawString(0, 16, "=");  
  }
    
  if(t_trend>=2&&t_trend<5){
      display.setTextAlignment(TEXT_ALIGN_LEFT); 
      display.setFont(fMon2038);
      display.drawString(0, 16, ";");  
  }

  if(t_trend==5){
      display.setTextAlignment(TEXT_ALIGN_LEFT); 
      display.setFont(fMon2038);
      display.drawString(0, 16, ":");  
  }
}

void AtmPressScreen() {
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(ArialMT_Plain_16);
  display.drawString(128, 0, meteoDataTime);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(fMon2038);
  display.drawString(128, 16, meteoAtmPress+"?");

  display.setTextAlignment(TEXT_ALIGN_LEFT); 
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "p"+String(p_trend));

  if(p_trend==-5){
      display.setTextAlignment(TEXT_ALIGN_LEFT); 
      display.setFont(fMon2038);
      display.drawString(0, 16, ">");  
  }
  
  if(p_trend>-5&&p_trend<=-2){
      display.setTextAlignment(TEXT_ALIGN_LEFT); 
      display.setFont(fMon2038);
      display.drawString(0, 16, "=");  
  }
    
  if(p_trend>=2&&p_trend<5){
      display.setTextAlignment(TEXT_ALIGN_LEFT); 
      display.setFont(fMon2038);
      display.drawString(0, 16, ";");  
  }

  if(p_trend==5){
      display.setTextAlignment(TEXT_ALIGN_LEFT); 
      display.setFont(fMon2038);
      display.drawString(0, 16, ":");  
  }
  
}

Screen screens[] = {TemperaturaScreen, AtmPressScreen};
int screenCount = (sizeof(screens) / sizeof(Screen));
long timeShowScreen = millis();

void loop() {
  // clear the display
  display.clear();

  // draw the current screens
  screens[screenNo]();

  display.setTextAlignment(TEXT_ALIGN_LEFT);  
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 54, statusLine);
  display.setTextAlignment(TEXT_ALIGN_RIGHT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(128, 54, String(REQ_GETMETEO_DURATION - counter - 1));

  // write the buffer to the display
  display.display();

  if (millis() - timeShowScreen > SCREEN_DURATION) {
    screenNo = (screenNo + 1) % screenCount;
    counter++;
    if (counter >= REQ_GETMETEO_DURATION)
    {
        counter = 0;
        getMeteoData();
      }
    timeShowScreen = millis();
  }
  delay(10);
}

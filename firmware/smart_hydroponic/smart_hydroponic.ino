 #define BLYNK_TEMPLATE_ID "YOUR_TEMPLATE_ID"
#define BLYNK_TEMPLATE_NAME "HYDROPONIC"
#define BLYNK_AUTH_TOKEN "YOUR_BLYNK_AUTH_TOKEN"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <HTTPClient.h>
#include "DHT.h"
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 19
OneWire oneWire(ONE_WIRE_BUS);  
DallasTemperature sensors(&oneWire);
#define DHTPIN 18 
#define DHTTYPE DHT11
#define waterpump 5
DHT dht(DHTPIN, DHTTYPE);

float calibration_value = 22.1;
int phval = 0; 
unsigned long int avgval; 
int buffer_arr[10],temp;
int noty = 0;
float ph_act;

char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "YOUR_WIFI_NAME";
char pass[] = "YOUR_WIFI_PASSWORD";

String GOOGLE_SCRIPT_ID = "YOUR_APPS_SCRIPT_DEPLOYMENT_ID";
String TIME = "";
String param;
unsigned long oldtime = 0;

BLYNK_WRITE(V0) {
 int buttonState1 = param.asInt();
  if (buttonState1 == 1){
     digitalWrite(waterpump,LOW);
  }
  else {
     digitalWrite(waterpump,HIGH);
  }
}

void setup()
{
  Serial.begin(9600);
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  sensors.begin();
  pinMode(34,INPUT);
  pinMode(35,INPUT);
  pinMode(waterpump,OUTPUT);
  digitalWrite(waterpump,HIGH);
}

void loop()
{
  Blynk.run();
  PH();
  sensors.requestTemperatures(); 
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  int waterlvl = analogRead(35);
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V2, h);
  Serial.print("Temp. Water: ");
  Serial.print(sensors.getTempCByIndex(0));
  Serial.print(" WATER LVL: ");
  Serial.print(waterlvl);
  Serial.print(" TEMP: ");
  Serial.print(t);
  Serial.print(" HUMI: ");
  Serial.println(h);
  Blynk.virtualWrite(V5, sensors.getTempCByIndex(0));
   if (millis() - oldtime > 15000) {
    oldtime = millis();
    param = "&temp=" + String(t);
    param += "&humi=" + String(h);
    param += "&ph=" + String(ph_act);
    Serial.println(param);
    write_to_google_sheet(param);
    delay(1000);
  }
  if(waterlvl < 500){
    Blynk.virtualWrite(V4, "NO WATER");
  }
  else if(waterlvl >= 500){
    Blynk.virtualWrite(V4, "HAVE WATER");
  }
}
void PH(){
     for(int i=0;i<10;i++) 
    { 
    buffer_arr[i]=analogRead(34);
    delay(30);
    }
    for(int i=0;i<9;i++)
    {
    for(int j=i+1;j<10;j++)
    {
    if(buffer_arr[i]>buffer_arr[j])
    {
    temp=buffer_arr[i];
    buffer_arr[i]=buffer_arr[j];
    buffer_arr[j]=temp;
    }
    }
    }
    avgval=0;
    for(int i=2;i<8;i++)
    avgval+=buffer_arr[i];
    float volt=(float)avgval*5.0/4095/6; 
    ph_act = -5.70 * volt + calibration_value;
    Serial.print("pH Val: ");
    Serial.print(ph_act);
    Blynk.virtualWrite(V3, ph_act);
    delay(1000);
}
void write_to_google_sheet(String params) {
   HTTPClient http;
   String url="https://script.google.com/macros/s/"+GOOGLE_SCRIPT_ID+"/exec?"+params;
   //Serial.print(url);
    Serial.println("Postring data to Google Sheet");
    //---------------------------------------------------------------------
    //starts posting data to google sheet
    http.begin(url.c_str());
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);
    int httpCode = http.GET();  
    Serial.print("HTTP Status Code: ");
    Serial.println(httpCode);
    //---------------------------------------------------------------------
    //getting response from google sheet
    String payload;
    if (httpCode > 0) {
        payload = http.getString();
        Serial.println("Payload: "+payload);     
    }
    //---------------------------------------------------------------------
    http.end();
}

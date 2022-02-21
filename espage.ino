
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFiMulti.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#define DHTPIN 2 //D4 pin
#define DHTTYPE DHT11
ESP8266WebServer server;
ESP8266WiFiMulti WiFiMulti;

float sensorValue;
float avg; //average output
int c; //count
DHT dht(DHTPIN, DHTTYPE);

//head and refresher of html page, set at "1" second
//programmed to flash memory to save space
char ind[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta http-equiv="refresh" content="1" charset="UTF-8"></meta>

)=====";
  
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  
  WiFi.mode(WIFI_STA);
  /*d1 mini pro seems to have problems with the standard
   * arduino wifi library, so I recommend using the
   * esp8266wifimulti library, namely addAP for
   * establishing connections*/
  WiFiMulti.addAP("SSID", "PASS");
  Serial.begin(115200);

  //LED will show once connection is established
  //HIGH and LOW are reversed when programming to esp8266
  while(WiFi.status()!=WL_CONNECTED)
  {
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_BUILTIN, LOW);
  
  server.on("/",[](){
  if(c == 360){c = 0;avg = 0;} //after an hour, reset the mean
  if(sensorValue < 0){sensorValue = sensorValue * -1;}
  if(sensorValue == sensorValue){avg += sensorValue;c++;}
  Serial.println(sensorValue);
  int hp = 5; //hex proportions
  int tp = 5; //text proportions
  float tol = 5; //value tolerance in %
  String hex = String(int(sensorValue * hp), HEX);
  //the rest of the html, such as title and bgcolor
  String sv2 = "<title>" + String(sensorValue) + "</title></head><body style='font-size:" + tp + "vw;color:#" + hex + "0000'><font face='arial'>" + String(sensorValue) + "℃ ";
  sv2 = sv2 + "<b style='font-size:" + int(tp/2) + "vw;'>";
  for(int i=0;i<sensorValue;i++){
    sv2 = sv2 + "☼";
  }
  sv2 = sv2 + "</b>";
  sv2 = sv2 + "<br><b style='font-size:" + tp/2 + "vw;'>Mean temp: " + avg/c + "℃<br>";
  if((sensorValue - (sensorValue * (tol/100))) > (avg/c)){sv2 = sv2 + "⇡ Warming";}
  if((sensorValue + (sensorValue * (tol/100))) < (avg/c)){sv2 = sv2 + "⇣ Cooling";}
  sv2 = sv2 + "</b></body></html>";
  char sv3[strlen(ind) + sv2.length()];
  //combine ind and sv2 so the html page is complete
  strcpy(sv3,ind);
  strcat(sv3,sv2.c_str());
  server.send_P(200,"text/html",sv3);
  });
  server.begin();
}

void loop()
{
  sensorValue = dht.readTemperature(); //more stable than using setup()
  server.handleClient();
}


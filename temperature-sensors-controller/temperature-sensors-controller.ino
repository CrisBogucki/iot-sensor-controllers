#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>

#define APP_NAME "CB Sensors Controller"
#define APP_VERSION "v. 0.0.1"

#define ONE_WIRE_BUS D4   
#define WIFI_SSID "WLAN-CENTER"
#define WIFI_PASSWORD "Tq9h39DjigFgREYH"


int sensors_count = 0;


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void wifi_connect()
{
    
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    console_print("Connecting to " + String(WIFI_SSID));
    while (WiFi.status() != WL_CONNECTED)
    {
        console_print(".");
        delay(300);
    }

    WiFi.setHostname(APP_NAME);
    String local_ip = WiFi.localIP().toString();
    console_println("");
    console_print("Connected with IP: " + local_ip);
    console_println("");
}

void init_sensors()
{
  sensors.begin();
  sensors_count = sensors.getDeviceCount();
  
  console_println("Locating devices...");
  console_println("Found " + String(sensors_count) + " sensors");
}

void read_sensors()
{
  sensors.requestTemperatures(); 

  for (int i = 0;  i < sensors_count;  i++)
  {
    Serial.print("Sensor ");
    Serial.print(i+1);
    Serial.print(" : ");
    float tempC = sensors.getTempCByIndex(i);
    Serial.print(tempC);
    Serial.print("°");//shows degrees character
    Serial.print("C  |  ");
    Serial.print(DallasTemperature::toFahrenheit(tempC));
    Serial.print("°");//shows degrees character
    Serial.println("F");
  }
  
  Serial.println("");
  delay(1000);
}

void setup() {
  
  Serial.begin(115200);
  
  console_println("");
  console_println("");
  
  console_println("=======================================");
  console_println("     " + String(APP_NAME) + " " + String(APP_VERSION));
  console_println("=======================================");
  
  init_sensors();
  read_sensors();
  wifi_connect();

}

void console_print(String a)
{
  Serial.print(a);
}

void console_println(String a)
{
  Serial.println(a);
}

void loop() {
  // put your main code here, to run repeatedly:
  read_sensors();
}

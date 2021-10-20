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
  sensors.begin();
  sensors_count = sensors.getDeviceCount();
  sensors.requestTemperatures(); 

  for (int i = 0;  i < sensors_count;  i++)
  {
    console_print("Sensor " + String(i+1) + " : ");

    DeviceAddress tempDeviceAddress;
    sensors.getAddress(tempDeviceAddress,i);
    float tempC = sensors.getTempCByIndex(i);
    if(tempC == -127.00)
      return;
      
    String tempF = String(DallasTemperature::toFahrenheit(tempC));
    
    console_print(String(tempC));
    console_print("°");//shows degrees character
    console_print("C  |  ");
    console_print(tempF);
    console_print("°");//shows degrees character
    console_print("F | ");
    print_address(tempDeviceAddress);

    console_println("");
  }

  if(sensors_count > 0)
  {
    console_println("");
    delay(1000);
  }
   
}

void setup() {
  
  Serial.begin(115200);
  
  console_println("");
  console_println("");
  
  console_println("=======================================");
  console_println("     " + String(APP_NAME) + " " + String(APP_VERSION));
  console_println("=======================================");
  
  init_sensors();
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

void print_address(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  read_sensors();
}

#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define APP_NAME "CB Sensors Controller"
#define APP_VERSION "v. 0.0.1"

#define ONE_WIRE_BUS D4   
#define WIFI_SSID "WLAN-CENTER"
#define WIFI_PASSWORD "Tq9h39DjigFgREYH"

int sensors_count = 0;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

String content;
ESP8266WebServer server(80);


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

    web_server();
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

void web_server()
{
  
  server.begin();
  
  console_print("idzie content 12");
  
  server.on("/", []()
  {
    IPAddress ip = WiFi.localIP();
     content = "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01//EN' 'http://www.w3.org/TR/html4/strict.dtd'>";
    content += "<html lang='en'>";
    content += "<head>";
        content += "<title>CB Sensors Controller</title>";
        content += "<style type='text/css'>";
            content += "html { font-family: 'Calibri', serif; background: #fff; color: #666; padding: 20px; }";
            content += "table{ width: 100%; margin: auto;}";
            content += "table tr td {width: 50%}";
            content += "table tr td:first-child{ text-align: right;}";
            content += "input{ width: 200px; border: 1px solid #666; color: #111; border-radius: 5px; padding: 5px }";
            content += "button{ width:200px; padding: 5px; border: 1px solid #ddd; border-radius: 3px }";
            content += "h3{ position: relative; color: #000; font-size: 22px; text-shadow: 2px 2px 10px #000}";
            content += "h3 > span { color: #2574be; font-size: 13px; position: absolute; margin-top: 20px; margin-left: -10px}";
            content += ".text-center{ text-align: center !important }";
        content += "</style>";
    content += "</head>";
    content += "<body>";
    content += "<form action='/'>";
        content += "<table>";
            content += "<tr><td colspan='2' class='text-center'><h3>SENSOR CONTROLLER <span>by CrisB</span></h3></td></tr>";
            content += "<tr><td>Host Name</td><td> : <label><input type='text' name='host_name' value='CB SENSOR CONTROLLER' required /></label></td></tr>";
            content += "<tr><td>SSID Name </td><td> : <label><input type='text' name='ssid_name' required /></label></td></tr>";
            content += "<tr><td>SSID Password </td><td> : <label><input type='password' name='ssid_pass' required /></label></td></tr>";
            content += "<tr><td>Token ID </td><td> : <label><input type='password' name='token_id' required /></label></td></tr>";
            content += "<tr><td>Sent Interval (sec)</td><td> : <label><input type='number' name='interval' required /></label></td></tr>";
            content += "<tr><td colspan='2' class='text-center'><br><br><button type='submit'>Update</button></td></tr>";
            content += "<tr><td colspan='2' class='text-center'><br>JSON VALUE<br>";
            content += "<pre style='text-align: left; width: 200px; margin-left: auto; margin-right: auto' xml:lang='text/json'></pre></td></tr>";
        content += "</table>";
    content += "</form>";
    content += "</body>";
    content += "</html>";
    server.send(200, "text/html", content);        
  });
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

void loop() {
  // put your main code here, to run repeatedly:
  //read_sensors();
  server.handleClient();
}

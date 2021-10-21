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


String scan_networks()
{
  String res = "";
      int n = WiFi.scanNetworks();
    if (n == 0)
    {
      res += "<table><tr><td><h3>No networks found!</h3></td></tr></table>";
    }
    else
    {
      res += "<div class='offset-lg-4 col-lg-4'><table><tr><td><ol>";
      res += "<li><h4>Available WiFi networks</h4></li>"
      for (int i = 0; i < n; ++i)
      {
        // Print SSID and RSSI for each network found
        res += "<li>";
        res += WiFi.SSID(i);
        res += "   (";
        res += WiFi.RSSI(i);
        res += " dBm = ";
        if (WiFi.RSSI(i)>-50)
        {res += "100";}
        else if (WiFi.RSSI(i)<-100)
        {res += "0";}
        else
        {res += 2*(WiFi.RSSI(i)+100);}
        res += (WiFi.encryptionType(i) == ENC_TYPE_NONE)?"%),   open ":"%),   protected ";
        res += "</li>";
      }
      res += "</ol></td></tr></table></div>";
    }

    return res;
}

void web_server()
{ 
  server.begin();
  
  server.on("/", []()
  {    
    IPAddress ip = WiFi.localIP();
     content = "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01//EN' 'http://www.w3.org/TR/html4/strict.dtd'>";
    content += "<html lang='en'>";
    content += "<head>";
        content += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
        content += "<title>CB Sensors Controller</title>";
        content += "<link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css'>";
        content += "<script src='https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.min.js'  type='text/javascript'></script>";
    content += "</head>";
    content += "<body class='container-sm'>";
    content += "<div class='mt-5 bd-brand-logos mb-3 offset-lg-4 col-lg-4'>";
        content += "<div class='bd-brand-item'><span class='h1'>SENSOR CONTROLLER</span></div>";
    content += "</div>";
    
        content += "<form action='settings.html'>";
            
            content += "<div class='mb-3 offset-lg-4 col-lg-4 '>";
                content += "<label for='host_name' class='form-label'>Host Name</label>";
                content += "<input type='text' class='form-control' id='host_name' placeholder='Enter host name' focus required>";
            content += "</div>";
            content += "<div class='mb-3 offset-lg-4 col-lg-4'>";
                content += "<label for='ssid_name' class='form-label'>SSID Name</label>";
                content += "<input type='text' class='form-control' id='ssid_name' placeholder='Enter ssid name' required>";
            content += "</div>";
            content += "<div class='mb-3 offset-lg-4 col-lg-4'>";
                content += "<label for='ssid_password' class='form-label'>SSID Password</label>";
                content += "<input type='password' class='form-control' id='ssid_password' placeholder='Enter ssid password' required>";
            content += "</div>";
            content += "<div class='mb-3 offset-lg-4 col-lg-4'>";
                content += "<label for='token_id' class='form-label'>Token ID</label>";
                content += "<input type='text' class='form-control' id='token_id' placeholder='Enter token id' required>";
            content += "</div>";
            content += "<div class='mb-3 offset-lg-4 col-lg-4'>";
                content += "<label for='interval' class='form-label'>Sent Interval (sec)</label>";
                content += "<input type='number' class='form-control' id='interval' placeholder='Enter interval in seconds' required>";
            content += "</div>";
        
            content += "<button type='submit' class='offset-lg-4 col-lg-4 col-12 btn btn-outline-primary'>Update</button>";
            
        content += "</form>";
        content += scan_networks();
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
  read_sensors();
  server.handleClient();
}

#include <OneWire.h>
#include <DallasTemperature.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <NTPClient.h>
#include <WifiUdp.h>

#define APP_NAME "CB Sensors Controller"
#define APP_VERSION " v0.0.1"

#define ONE_WIRE_BUS D4

char* HOST_NAME = "CB Sensors Controller";
String WIFI_SSID = "";
String WIFI_PASSWORD = "";
String SERVICE_URL = "";
String TOKEN_ID = "8ef050eb-9397-4ffb-b206-339e6fd85d7b";
String EpochTime = "";
int WIFI_STATUS = 0;

int sensors_count = 0;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

String content;
ESP8266WebServer server(80);



WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
//Week Days
String weekDays[7]={"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//Month names
String months[12]={"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};



void wifi_connect() {
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    console_print("Connecting to " + String(WIFI_SSID));
    while (WiFi.status() != WL_CONNECTED) {
        console_print(".");
        delay(300);
    }
    WIFI_STATUS = 1;
    WiFi.setHostname(HOST_NAME);
    String local_ip = WiFi.localIP().toString();
    console_println("");
    console_print("Connected with IP: " + local_ip);
    console_println("");

    web_server();
}

void init_sensors() {
    sensors.begin();
    sensors_count = sensors.getDeviceCount();

    console_println("Locating devices...");
    console_println("Found " + String(sensors_count) + " sensors");
}

String read_sensors_to_console_and_html() {
    sensors.begin();
    sensors_count = sensors.getDeviceCount();
    sensors.requestTemperatures();

    String content = "";

    for (int i = 0; i < sensors_count; i++) {
        console_print("Sensor " + String(i + 1) + " : ");

        DeviceAddress tempDeviceAddress;
        sensors.getAddress(tempDeviceAddress, i);
        float tempC = sensors.getTempCByIndex(i);
        if (tempC != -127.00) {
            String tempF = String(DallasTemperature::toFahrenheit(tempC));

            console_print(String(tempC));
            console_print("°");//shows degrees character
            console_print("C  |  ");
            console_print(tempF);
            console_print("°");//shows degrees character
            console_print("F | ");
            print_address(tempDeviceAddress);  

            console_println("");

            content += "<div class='offset-lg-4 col-lg-4 '>";
            content +=
                    "<label for='sensor' class='form-label'>Sensor " + String(i + 1) + " | " + String(tempC) + "'C | " +
                    tempF + "'F </label>";
            content += "<div class='offset-lg-4 col-lg-4 '> <small> sensor id: " + addressToString(tempDeviceAddress) +
                       " </small></div>";
            content += "</div>";

        }
    }

    if (sensors_count > 0) {
        console_println("");
    }

    return content;
}

String read_sensors_as_JSON() {
    sensors.begin();
    sensors_count = sensors.getDeviceCount();
    sensors.requestTemperatures();

    String result = "{";

    result += "\"token\" : \"" + TOKEN_ID +"\",";
    result += "\"epochTime\" : \"" + EpochTime +"\",";
    result += "\"value\":[";

    for (int i = 0; i < sensors_count; i++) {
        DeviceAddress tempDeviceAddress;
        sensors.getAddress(tempDeviceAddress, i);
        float tempC = sensors.getTempCByIndex(i);

        if (tempC != -127.00) {
            if (i > 0)
                result += ",";

            result += "{ ";
            String tempF = String(DallasTemperature::toFahrenheit(tempC));
            result += "\"address\" : \"" + addressToString(tempDeviceAddress) + "\"";
            result += ", \"tempC\" : \"" + String(tempC) +"\"";
            result += ", \"tempF\" : \"" + tempF +"\"";
            result += " }";
        }
    }

    result += "]}";

    if (sensors_count > 0) {
        console_println("");
        delay(500);
    }

    return result;

}

void console_print(String a) {
    Serial.print(a);
}

void console_println(String a) {
    Serial.println(a);
}

void print_address(DeviceAddress deviceAddress) {
    for (uint8_t i = 0; i < 8; i++) {
        if (deviceAddress[i] < 16) Serial.print("0");
        Serial.print(deviceAddress[i], HEX);
    }
}

String addressToString(DeviceAddress deviceAddress) {
    String res = "";
    for (uint8_t i = 0; i < 8; i++) {
        if (deviceAddress[i] < 16) res += "0";
        res += String(deviceAddress[i], HEX);
    }
    return res;
}

String scan_networks() {
    String res = "";
    int n = WiFi.scanNetworks();
    if (n == 0) {
        res += "<table><tr><td><h3>No networks found!</h3></td></tr></table>";
    } else {
        res += "<div class='offset-lg-4 col-lg-4'><span class='h1'>Available WiFi networks</span>";
        for (int i = 0; i < n; ++i) {
            res += "<span>";
            res += WiFi.SSID(i);
            res += "   (";
            res += WiFi.RSSI(i);
            res += " dBm = ";
            if (WiFi.RSSI(i) > -50) {
                res += "100";
            } else if (WiFi.RSSI(i) < -100) {
                res += "0";
            } else {
                res += 2 * (WiFi.RSSI(i) + 100);
            }
            res += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? "%),   open " : "%),   protected ";
            res += "</span>";
        }
        res += "</div>";
    }

    return res;
}

void web_server() {
    server.begin();

    server.on("/", []() {
        IPAddress ip = WiFi.localIP();
        content = "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01//EN' 'http://www.w3.org/TR/html4/strict.dtd'>";
        content += "<html lang='en'>";
        content += "<head>";
        content += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
        content += "<title>" + String(APP_NAME) + "</title>";
        content += "<link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css'>";
        content += "<script src='https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.min.js'  type='text/javascript'></script>";
        content += "</head>";
        content += "<body class='container-sm'>";
        content += "<div class='mt-5 bd-brand-logos mb-3 offset-lg-4 col-lg-4'>";
        content += "<div class='bd-brand-item'><span class='h1'>" + String(APP_NAME) + "</span> <small >" +
                   String(APP_VERSION) + "</small> </div>";
        content += "</div>";

        content += "<form action='/save' method='post'>";

        content += "<div class='mb-3 offset-lg-4 col-lg-4 '>";
        content += "<label for='host_name' class='form-label'>Host Name</label>";
        content +=
                "<input type='text' class='form-control' name='host_name' id='host_name' value='" + String(HOST_NAME) +
                "' placeholder='Enter host name' focus required maxlength='30'>";
        content += "</div>";
        content += "<div class='mb-3 offset-lg-4 col-lg-4'>";
        content += "<label for='ssid_name' class='form-label'>SSID Name</label>";
        content +=
                "<input type='text' class='form-control' name='ssid_name' id='ssid_name' value='" + String(WIFI_SSID) +
                "' placeholder='Enter ssid name' required maxlength='30'>";
        content += "</div>";
        content += "<div class='mb-3 offset-lg-4 col-lg-4'>";
        content += "<label for='ssid_password' class='form-label'>SSID Password</label>";
        content += "<input type='password' class='form-control' name='ssid_password' id='ssid_password' value='" +
                   String(WIFI_PASSWORD) + "' placeholder='Enter ssid password' required maxlength='30'>";
        content += "</div>";


        content += "<div class='mb-3 offset-lg-4 col-lg-4'>";
        content += "<label for='service_url' class='form-label'>Service URL</label>";
        content += "<input type='text' class='form-control' name='service_url' id='service_url' value='" +
                   String(SERVICE_URL) + "' placeholder='Enter service url' required maxlength='100'>";
        content += "</div>";
        content += "<div class='mb-3 offset-lg-4 col-lg-4'>";
        content += "<label for='token_id' class='form-label'>Token ID</label>";
        content += "<input type='text' class='form-control' name='token_id' id='token_id' value='" + String(TOKEN_ID) +
                   "' placeholder='Enter token id' required maxlength='100'>";
        content += "</div>";

        content += "<input type='submit' class='offset-lg-4 col-lg-4 col-12 btn btn-outline-primary' value='Update'/>";

        content += "</form>";
        content += scan_networks();
        content += "</body>";
        content += "</html>";

        server.send(200, "text/html", content);
    });

    server.on("/save", []() {
        int arg_count = 5;

        for (int i = 0; i < arg_count; i++) {
            console_println(server.arg(i));
         
        }

        String content = "Zapisamo do pamieci";

        server.send(200, "text/text", content);
    });

    server.on("/preview", []() {
        IPAddress ip = WiFi.localIP();
        content = "<!DOCTYPE HTML PUBLIC '-//W3C//DTD HTML 4.01//EN' 'http://www.w3.org/TR/html4/strict.dtd'>";
        content += "<html lang='en'>";
        content += "<head>";
        content += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
        content += "<title>" + String(APP_NAME) + "</title>";
        content += "<link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/css/bootstrap.min.css'>";
        content += "<script src='https://cdn.jsdelivr.net/npm/bootstrap@5.1.3/dist/js/bootstrap.min.js'  type='text/javascript'></script>";
        content += "</head>";
        content += "<body class='container-sm'>";
        content += "<div class='mt-5 bd-brand-logos mb-3 offset-lg-4 col-lg-4'>";
        content += "<div class='bd-brand-item'><span class='h1'>" + String(APP_NAME) + "</span> <small>" +
                   String(APP_VERSION) + "</small> </div>";
        content += "</div>";


        content += read_sensors_to_console_and_html();


        content += "</body>";
        content += "</html>";

        server.send(200, "text/html", content);
    });
    server.on("/data", []() {
        String _json = "";
        _json = read_sensors_as_JSON();
        server.send(200, "application/json", _json);
    });
}


    
void send_data()
{
  httpPost(SERVICE_URL, read_sensors_as_JSON());  
}

String httpPost(String url, String payload)
{
  WiFiClientSecure client;
  client.setInsecure();
  HTTPClient https;
  String result = "";
  
  if (https.begin(client, url)) {  
    https.headers();
    int httpCode = https.POST(payload);

    if (httpCode > 0) {
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        result = https.getString();
        console_println("Request HTTPS -> OK");
      }
    } else {
      console_println("Request HTTPS -> ERROR " + String(https.errorToString(httpCode).c_str()));
    }
  }
  https.end();
  
  return result;
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
  timeClient.begin();
  timeClient.setTimeOffset(0);
}

void loop() {
  timeClient.update();
  if (WIFI_STATUS == 1) {
    unsigned long epochTime = timeClient.getEpochTime();
    EpochTime = String(epochTime);
      read_sensors_to_console_and_html();
      send_data();
  }

  

  server.handleClient();
}

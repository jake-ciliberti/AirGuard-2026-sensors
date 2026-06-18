#include <WiFiS3.h>

const char ssid[] = "JTR";
const char pass[] = "FlexibleDoor84";

int status = WL_IDLE_STATUS;

WiFiClient client;

char server[] = "example.org";
//IPAddress server(64,131,82,241);

unsigned long lastConnectionTime_ms = 0;
const unsigned long postingInterval_ms = 10L * 1000L;
int keyIndex = 0;

NTPClient timeClient(ntpUDP);

void setup() {
  Serial.begin(115200);

  // WIFI SETUP
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  
  // TIME SETUP
  timeClient.begin();

  // FLUXTEQ SETUP

  // MKR ENV SHIELD SETUP

  // OZONE MQ131 SETUP

  // SPARKFUN QWIIC CO2 SETUP

  // SEN55-SDN-T SETUP

  // MICROPHONE SETUP
}

void read_request() {
  uint32_t received_data_num = 0;

  while (client.available()) {
    /* actual data reception */
    char c = client.read();
    /* print data to serial port */
    Serial.print(c);
    /* wrap data to 80 columns*/
    received_data_num++;
    if(received_data_num % 80 == 0) { 
      
    }
    
  }  
}

void loop() {
  read_request();

  String jsonData = constructJSON();

  if (millis() - lastConnectionTime_ms > postingInterval_ms) {
    httpRequest(jsonData);
  }
  delay(20000);
}

void httpRequest(String jsondata) {
  // close any connection before send a new request.
  // This will free the socket on the NINA module
  client.stop();

  // if there's a successful connection:
  if (client.connect(server, 8080)) {  // client and server might both be able to use 80; test to see if that's possible
    Serial.println("connecting...");
    // send the HTTP GET request:
    client.println("POST /data HTTP/1.1");
    client.println("Host: example.org");
    client.println("User-Agent: ArduinoWiFi/1.1"); // Change per sensor so that the server knows where all the data comes from
    clinet.println("Content-Type: application.json")
    client.print("Content-Length: ");
    client.print(jsonData.length());
    client.print("\n\n");

    client.print(jsonData);
    
    client.println("Connection: close");
    client.println();
    // note the time that the connection was made:
    lastConnectionTime_ms = millis();
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

String constructJSON() {
  timeClient.update();
  // BRAND_SENSOR_MEASUREDQUANITITY: DATA
  String result = "{\"time\":%d}", timeClient.getEpochTime(); // RETURNS UNIX TIME; ADDS EPOCH
}
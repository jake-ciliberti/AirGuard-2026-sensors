#include <Arduino.h>
#include <Arduino_MKRENV.h>
#include <ArduinoJson.h>
#include <NTPClient.h>
#include <pas-co2-ino.hpp>
#include <SensirionI2cSps30.h>
#include <WiFiS3.h>
#include <WiFiUdp.h>
#include <Wire.h>

/* 
 * The sensor supports 100KHz and 400KHz. 
 * You hardware setup and pull-ups value will
 * also influence the i2c operation. You can 
 * change this value to 100000 in case of 
 * communication issues.
 */
// TODO: check to make sure this does not mess up the other sensors that use WIRE
#define I2C_FREQ_HZ 400000

const char ssid[] = "JTR";
const char pass[] = "FlexibleDoor84";

int status = WL_IDLE_STATUS;

WiFiClient wifi;
WiFiUDP ntpUDP;

PASCO2Ino cotwo;

int16_t co2ppm;
Error_t err;

const int16_t SEN55_ADDRESS = 0x69;

char server[] = "example.org";
//IPAddress server(64,131,82,241);

unsigned long lastConnectionTime_ms = 0;
const unsigned long postingInterval_ms = 10L * 1000L;
int keyIndex = 0;

NTPClient timeClient(ntpUDP);
JsonDocument doc;

void setup() {
  Serial.begin(115200);
  // might be the wrong wire. work on this if the wires don't work https://docs.arduino.cc/language-reference/en/functions/communication/wire/#default-i2c-pins
  Wire.begin();

  // WIFI SETUP
  while (!Serial);

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

  if (!ENV.begin()) {
    Serial.println("Failed to initialize MKR ENV Shield!");
    while (1);
  }

  // OZONE MQ131 SETUP

  // SPARKFUN QWIIC CO2 SETUP

  // SEN55-SDN-T SETUP
  /*Wire.setClock(I2C_FREQ_HZ);

  Wire.beginTransmission(SEN55_ADDRESS);
  Wire.write(0x00);
  Wire.write(0x21);
  Wire.endTransmission();

  err = cotwo.begin();
  if(XENSIV_PASCO2_OK != err)
  {
    Serial.print("initialization error: ");
    Serial.println(err);
  }*/

  // MICROPHONE SETUP
}

void read_request() {
  uint32_t received_data_num = 0;

  while (wifi.available()) {
    /* actual data reception */
    char c = wifi.read();
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

  delay(20000);

  if (millis() - lastConnectionTime_ms > postingInterval_ms) {
    char buffer[2048];
    constructJSON(buffer);
    httpRequest(buffer);
  }
}

void httpRequest(char *buffer) {
  // close any connection before send a new request.
  // This will free the socket on the NINA module
  wifi.stop();

  // if there's a successful connection:
  if (wifi.connect(server, 8080)) {  // client and server might both be able to use 80; test to see if that's possible
    Serial.println("connecting...");
    // send the HTTP GET request:
    wifi.println("POST /measurements HTTP/1.1");
    wifi.println("Host: example.org");
    wifi.println("User-Agent: ArduinoWiFi/1.1"); // Change per sensor so that the server knows where all the data comes from
    wifi.println("Content-Type: application.json");
    wifi.print("Content-Length: ");
    wifi.print(measureJson(doc));
    wifi.print("\n\n");

    wifi.print(buffer);
    
    wifi.println("Connection: close");
    wifi.println();
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

void constructJSON(char *buffer) {
  // TIME
  timeClient.update();

  // SEN55
  Wire.requestFrom(SEN55_ADDRESS, 24);
  int counter = 0;
  
  // while (Wire.available()) {
  //   // PM1.0 to PM10 are unscaled unsigned integer values in ug / um3
  //   // VOC level is a signed int and scaled by a factor of 10 and needs to be divided by 10
  //   // humidity is a signed int and scaled by 100 and need to be divided by 100
  //   // temperature is a signed int and scaled by 200 and need to be divided by 200
  //   data[counter++] = Wire.read();
  // }

  // real construction
  // BRAND_SENSOR_MEASUREDQUANITITY: DATA
  doc["time"] = timeClient.getEpochTime();
  doc["mkr_env_temperature"] = ENV.readTemperature();
  doc["mkr_env_humidity"] = ENV.readHumidity();
  doc["mkr_env_pressure"] = ENV.readPressure();
  doc["mkr_env_illuminance"] = ENV.readIlluminance();
  doc["mkr_env_uvindex"] = ENV.readUVIndex();
  // doc["sensirion_sen55_pm1p0"] = float((uint16_t)data[0] << 8 | data[1]) / 10;
  // doc["sensirion_sen55_pm2p5"] = float((uint16_t)data[3] << 8 | data[4]) / 10;
  // doc["sensirion_sen55_pm4p0"] = float((uint16_t)data[6] << 8 | data[7]) / 10;
  // doc["sensirion_sen55_pm10p0"] = float((uint16_t)data[9] << 8 | data[10]) / 10;
  // doc["sensirion_sen55_humidity"] = float((uint16_t)data[12] << 8 | data[13]) / 10;
  // doc["sensirion_sen55_temperature"] = float((uint16_t)data[15] << 8 | data[16]) / 10;
  // doc["sensirion_sen55_voc"] = float((uint16_t)data[18] << 8 | data[19]) / 100;
  // doc["sensirion_sen55_nox"] = float((uint16_t)data[21] << 8 | data[22]) / 200;

  serializeJson(doc, buffer);
}
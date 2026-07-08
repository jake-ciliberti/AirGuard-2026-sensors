#include <Arduino.h>
#include <Arduino_MKRENV.h>
#include <ArduinoJson.h>
#include <MQ131.h>
#include <NTPClient.h>
#include <pas-co2-ino.hpp>
#include <SensirionI2cSps30.h>
#include <SparkFun_STC3x_Arduino_Library.h>
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

const int16_t SEN55_ADDRESS = 0x69;

PASCO2Ino cotwo;

int16_t co2ppm;
Error_t err;

char remote[] = "test.jakeciliberti.com";
//IPAddress remote(10,214,12,93);
int port = 80;

char buffer[1024] = "Starting sensors!";

unsigned long lastConnectionTime_ms = 0;
const unsigned long postingInterval_ms = 19L * 1000L;
int keyIndex = 0;

NTPClient timeClient(ntpUDP);
JsonDocument doc;

WiFiServer server(80);

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // WIFI SETUP
  while (!Serial);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
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
    while (true);
  }

  // OZONE MQ131 SETUP
  MQ131.begin(2, A0, LOW_CONCENTRATION, 1000000);
  // NOTE: uncomment the following lines for active calibration
  // Serial.println("Calibration in progress...");
  // MQ131.calibrate();
  // Serial.println("Calibration parameters:");
  // Serial.print("R0 = ");
  // Serial.print(MQ131.getR0());
  // Serial.println(" Ohms");
  // Serial.print("Time to heat = ");
  // Serial.print(MQ131.getTimeToRead());
  // Serial.println(" s");

  // SENSIRION SEN55 Setup

  // for more information: https://github.com/Sensirion/arduino-snippets/blob/main/SEN5x_I2C_switch_measurement_mode/SEN5x_I2C_switch_measurement_mode.ino

  Wire.beginTransmission(SEN55_ADDRESS);
  Wire.write(0x00);
  Wire.write(0x21);
  Wire.endTransmission();

  // SPARKFUN QWIIC CO2 SETUP

  /* err = cotwo.begin();
  if(XENSIV_PASCO2_OK != err)
  {
    Serial.print("initialization error: ");
    Serial.println(err);
  }*/

  // MICROPHONE SETUP

  // SERVER
  server.begin();

  constructJSON();

  Serial.println("Done setting up");

  printWifiStatus();
}

void loop() {
  localServer();

  if (millis() - lastConnectionTime_ms > postingInterval_ms) {
    Serial.println("Getting sensor data");
    constructJSON();
    size_t size = measureJson(doc);
    httpRequest(size);
  }
}

void localServer() {
  WiFiClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an HTTP request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the HTTP request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard HTTP response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: application/json");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println();
          client.println(buffer);
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);

    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

void httpRequest(size_t size) {
  // close any connection before send a new request.
  // This will free the socket on the NINA module
  Serial.println(buffer);
  Serial.println(size);
  Serial.println("Starting HTTP connection");
  wifi.stop();

  // if there's a successful connection:
  if (wifi.connect(remote, port)) {  // client and server might both be able to use 80; test to see if that's possible
    Serial.println("connecting...");
    // send the HTTP GET request:
    wifi.println("POST /measurements HTTP/1.1");
    wifi.print("Host: ");
    wifi.println(remote); // needs to be casted to String if using a numerical IP
    wifi.println("User-Agent: ArduinoWiFi/1.1"); // Change per sensor so that the server knows where all the data comes from
    wifi.println("Content-Type: application.json");
    wifi.print("Content-Length: ");
    wifi.print(size);
    wifi.print("\n\n");

    wifi.print(buffer);
    
    wifi.println("Connection: close");
    wifi.println();
    // note the time that the connection was made:
    lastConnectionTime_ms = millis();
    Serial.println("connected");
  } else {
    // if you couldn't make a connection:
    Serial.println("connection failed");
  }
}

void constructJSON() {
  // TIME
  timeClient.update();

  // MQ131
  // MQ131.sample();

  // // SEN55
  uint16_t pm1p0, pm2p5, pm4p0, pm10p0;
  int16_t voc, nox, humidity, temperature;
  uint8_t data[24], counter;

  Wire.beginTransmission(SEN55_ADDRESS);
  Wire.write(0x03);
  Wire.write(0xC4);
  Wire.endTransmission();

  delay(20);

  Wire.requestFrom(SEN55_ADDRESS, 24);
  counter = 0;
  
  while (Wire.available()) {
    // PM1.0 to PM10 are unscaled unsigned integer values in ug / um3
    // VOC level is a signed int and scaled by a factor of 10 and needs to be divided by 10
    // humidity is a signed int and scaled by 100 and need to be divided by 100
    // temperature is a signed int and scaled by 200 and need to be divided by 200
    data[counter++] = Wire.read();
  }

  pm1p0 = (uint16_t)data[0] << 8 | data[1];
  pm2p5 = (uint16_t)data[3] << 8 | data[4];
  pm4p0 = (uint16_t)data[6] << 8 | data[7];
  pm10p0 = (uint16_t)data[9] << 8 | data[10];
  humidity = (uint16_t)data[12] << 8 | data[13];
  temperature = (uint16_t)data[15] << 8 | data[16];
  voc = (uint16_t)data[18] << 8 | data[19];
  nox = (uint16_t)data[21] << 8 | data[22];

  // real construction
  // BRAND_SENSOR_MEASUREDQUANITITY: DATA

  doc["time"] = timeClient.getEpochTime(); // gets epoch time in seconds with no offset

  // MKR ENV SHIELD
  doc["mkr_env_temperature"] = ENV.readTemperature(); // celsius
  doc["mkr_env_humidity"] = ENV.readHumidity();
  doc["mkr_env_pressure"] = ENV.readPressure(); // kPa
  doc["mkr_env_illuminance"] = ENV.readIlluminance(); // lx
  doc["mkr_env_uvindex"] = ENV.readUVIndex();

  // FLUXTEQ

  // MQ131
  // doc["soldered_mq131_ozone_ppm"] = MQ131.getO3(PPM);
  // doc["soldered_mq131_ozone_ppb"] = MQ131.getO3(PPB);
  // doc["soldered_mq131_ozone_mg_m3"] = MQ131.getO3(MG_M3);
  // doc["soldered_mq131_ozone_ug_m3"] = MQ131.getO3(UG_M3);

  // SEN55
  doc["sensirion_sen55_pm1p0"] = float(pm1p0) / 10; // ug / um3
  doc["sensirion_sen55_pm2p5"] = float(pm2p5) / 10; // ug / um3
  doc["sensirion_sen55_pm4p0"] = float(pm4p0) / 10; // ug / um3
  doc["sensirion_sen55_pm10p0"] = float(pm10p0) / 10; // ug / um3
  doc["sensirion_sen55_voc"] = float(voc) / 10;
  doc["sensirion_sen55_nox"] = float(nox) / 10;
  doc["sensirion_sen55_humidity"] = float(humidity) / 100;
  doc["sensirion_sen55_temperature"] = float(temperature) / 200; // celsius

  serializeJson(doc, buffer, 1024);
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
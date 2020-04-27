#include <WiFi.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include <AD9851.h>

#define RST   23
#define DATA  5
#define FQ    19
#define CLK   18
DDS dds = dds_init(RST, DATA, FQ, CLK);

/* Set these to your desired credentials. */
const char *ssid = "ESPap";
const char *password = "thereisnospoon";
const int portNo = 53000;
WiFiServer server(portNo);
WiFiClient client;

SSD1306Wire display(0x3c, 21, 22);
String displayBuffer[4];

void mySetup();
void myLoop();
void updateDisplay();
String process(String str);

void setup() {
  Serial.begin(57600);
  display.init();
  
  mySetup();

  Serial.print(String("\r\nConnecting to ") + ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\r\nWiFi connected");  
  server.begin();
  Serial.println("Server started (" + WiFi.localIP().toString() + ":" + portNo + ")");  

  displayBuffer[0] = WiFi.localIP().toString();
  displayBuffer[1] = String(portNo);
  updateDisplay();
}

void loop() {
  myLoop();
  
  if (!client.connected()) {
    // try to connect to a new client
    client = server.available();
    if (client.connected()) {
      Serial.println("connected to " + client.remoteIP().toString() + ":" + client.remotePort());  
    }
  } else {
    // read data from the connected client
    if (client.available()) {
      String inputString = client.readStringUntil('\n');
      inputString.trim();
      String outputString = process(inputString);
      if (outputString != "") {
        client.println(outputString);
      }
    }
  }
}

void mySetup() {
  dds_reset(dds);

  process("F:1000000");
}

void myLoop() {

}

void updateDisplay() {
  display.clear();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  for (int i = 0; i < 4; i++) {
    display.drawString(0, i * 16, displayBuffer[i]);
  }
  display.display();
}

String process(String str) {
  //Serial.println(str);
  
  if (str.startsWith("F:")) {
    int f = str.substring(2).toInt();
    writeFreq(dds, f);
    
    char buf[32];
    sprintf(buf, "%0.6fMHz", f / 1e6);
    //Serial.println(buf);
    displayBuffer[3] = buf;
    updateDisplay();
  }
  
  return "";
}

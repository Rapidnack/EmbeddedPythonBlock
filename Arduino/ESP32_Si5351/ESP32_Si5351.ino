#include <WiFi.h>
#include <Wire.h>
#include "SSD1306Wire.h"
#include "si5351.h"

Si5351 si5351;

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
  // Initialize the Si5351
  if(!si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0))
  {
    Serial.println("Device not found on I2C bus!");
    displayBuffer[0] = "Device not found";
    displayBuffer[1] = "on I2C bus!";
    updateDisplay();
    while (1);
  }
  
  process("C0:0");
  process("C1:0");
  process("C2:0");

  si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
  si5351.drive_strength(SI5351_CLK1, SI5351_DRIVE_8MA);
  si5351.drive_strength(SI5351_CLK2, SI5351_DRIVE_8MA);
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
  
  if (str.startsWith("C0:") || str.startsWith("C1:") || str.startsWith("C2:")) {
    String clk = str.substring(0, 2);
    enum si5351_clock c;
    if (clk == "C0") c = SI5351_CLK0;
    else if (clk == "C1") c = SI5351_CLK1;
    else c = SI5351_CLK2;
    
    long f = str.substring(3).toInt();
    if (f == 0) {
      si5351.output_enable(c, 0);
    } else {
      si5351.set_freq(f * 100ULL, c);
      si5351.output_enable(c, 1);
    }
    
    char buf[32];
    if (f == 0) {
      buf[0] = '\0';      
    } else {
      sprintf(buf, "%s:%0.6fM", clk, f / 1e6);
    }
    //Serial.println(buf);
    if (clk == "C0") displayBuffer[3] = buf;
    else if (clk == "C1") displayBuffer[2] = buf;
    else if (clk == "C2") displayBuffer[1] = buf;
    updateDisplay();
  }
  
  return "";
}

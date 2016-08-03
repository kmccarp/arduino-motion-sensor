#include <ESP8266WiFi.h>

char ssid[] = "ssid";
char pass[] = "pass";
char port[] = ":none";
int status = WL_IDLE_STATUS;

char server[] = "noneya";
WiFiClient client;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }

  wifiConnect();

  Serial.print("Connected!");
}

void loop() {
  // check the network connection once every two minutes:
  postSignalStrength();
  delay(120000);
}

void wifiConnect() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connecting to WPA SSID: ");
    Serial.println(ssid);
    WiFi.begin(ssid, pass);

    Serial.print("Connection status: ");
    Serial.println(WiFi.status());
    
    delay(5000);
  }

  // connect to server
  if (!client.connected()) {
    client.stop();
  }
  if (!client.connect(server, 3000)) {
    Serial.print("Could not connect to ");
    Serial.println(server);
    while(true);
  }
}

void postSignalStrength() {
  // check wifi connection status
  wifiConnect();
  
  Serial.println("Posting RSSI (signal strength)");
  client.print("POST /temperature_entries.json?temperaturef=");
  client.print(WiFi.RSSI());
  client.println(" HTTP/1.1");
  
  client.print("Host: ");
  client.print(server);
  client.println(port);

  client.println("content-length: 0");
  client.println();

  Serial.println("Posted!");

  // now read from the client
  while (client.available()) {
    char c = client.read();
    Serial.write(c);
  }
}


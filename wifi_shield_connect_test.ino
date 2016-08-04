#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>

// constants
const char ssid[] = "";
const char pass[] = "";
const int status = WL_IDLE_STATUS;
const char server[] = "";
const char port[] = "";
const int timeBetweenSignalPosts = 60 * 1000 * 2;
const int maxWifiConnectWait = 5 * 1000;

// pins
const int sensorPin = D2;

// state
int sensorState = LOW; // LOW represents no motion, HIGH represents motion
int sensorVal = 0;
int sensorMotionDuration = 0;
int timeSinceLastSignalPost = 0;
int wifiConnectWait = 0;

// connector
ESP8266WiFiMulti WiFiMulti;
WiFiClient client;

// functions
int wifiConnect();
void postReset();
void checkForMotion();

void setup() {
  Serial.begin(9600);
  pinMode(sensorPin, INPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  WiFiMulti.addAP(ssid, pass);
  while (!Serial);

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true);
  }

  postReset();
}

void loop() {
  // check the network connection once every two minutes:
  checkForMotion();
}

int wifiConnect() {
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(100);
  }

  return 1;
}

void post(const char *host, const char *port, const char *path, const char *params) {
  wifiConnect();
  
  HTTPClient http;
  char url[80];
  strcpy(url, "http://");
  strcat(url, host);
  strcat(url, ":");
  strcat(url, port);
  strcat(url, "/");
  strcat(url, path);
  http.begin(url);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  if (strlen(params) == 0) {
    http.addHeader("content-length", "0");
  }
  
  Serial.printf("[HTTP] Posting to %s\n", url);
  int httpCode = http.POST(params);
  if (httpCode != HTTP_CODE_OK && httpCode != HTTP_CODE_CREATED) {
    Serial.printf("[HTTP] Post failed! Error: %d : %s\n", httpCode, http.errorToString(httpCode).c_str());
  }
}

void postReset() {
  post(server, "3000", "resets.json", "");
}

void postSignalStrength() {
  char params[100];
  sprintf(params, "temperaturef=%d", WiFi.RSSI());
  post(server, "3000", "temperature_entries.json", params);
}

void postMotionDuration() {
  char params[100];
  sprintf(params, "duration_seconds=%d", sensorMotionDuration);
  post(server, "3000", "motions.json", params);
}

void advancedDelay(int delayMs) {
  timeSinceLastSignalPost += delayMs;

  if (sensorState == HIGH) {
    sensorMotionDuration += delayMs;
  }

  delay(delayMs);
}

void checkForMotion() {
  // check wifi connection status
  wifiConnect();

  if (timeSinceLastSignalPost >= timeBetweenSignalPosts) {
    postSignalStrength();
    timeSinceLastSignalPost = 0;
  }

  sensorVal = digitalRead(sensorPin);   // read sensor value
  if (sensorVal == HIGH) {           // check if the sensor is HIGH
    advancedDelay(100);                // delay 100 milliseconds

    if (sensorState == LOW) {
      Serial.println("Motion detected!");
      digitalWrite(LED_BUILTIN, LOW);
      sensorMotionDuration = 0;
      sensorState = HIGH;       // update variable state to HIGH
    }
  } else {
    advancedDelay(200);             // delay 200 milliseconds

    if (sensorState == HIGH) {
      Serial.printf("Motion stopped after %d seconds.\n", sensorMotionDuration);
      digitalWrite(LED_BUILTIN, HIGH);
      sensorState = LOW;       // update variable state to LOW
      postMotionDuration();
    }
  }

}


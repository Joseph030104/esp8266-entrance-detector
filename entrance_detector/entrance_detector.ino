/*
 * Entrance Detector with LINE Alerts
 *
 * An HC-SR04 ultrasonic sensor watches a doorway. When the measured distance
 * drops below a threshold, the ESP8266 calls an IFTTT webhook, and IFTTT
 * sends a message to LINE.
 *
 * Board:     NodeMCU ESP8266 (Arduino IDE, ESP8266 board package)
 * Libraries: ESP8266WiFi (bundled with the ESP8266 core), Ultrasonic
 *
 * Fill in the four placeholders below before uploading.
 * Never commit your real Wi-Fi password or IFTTT key.
 */

#include <ESP8266WiFi.h>
#include <Ultrasonic.h>

#define TRIGGER 5   // NodeMCU D1
#define ECHO    4   // NodeMCU D2

Ultrasonic ultrasonic(TRIGGER, ECHO);

// ---- Configuration (replace the placeholders) ----
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

const char* host      = "maker.ifttt.com";   // IFTTT host
const char* eventName = "someone_pass";      // IFTTT webhook event name
const char* key       = "YOUR_IFTTT_KEY";    // IFTTT webhook key
// --------------------------------------------------

void setup() {
  Serial.begin(9600);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {   // keep retrying until connected
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected!");
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());           // show the IP address
}

void loop() {
  int dis = ultrasonic.distanceRead();      // read distance
  Serial.print("Distance: ");
  Serial.print(dis);
  Serial.println(" CM");

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  if (dis < 30) {                           // someone is close to the sensor
    Serial.print("connecting to ");
    Serial.println(host);

    String url = "/trigger/";               // build the webhook URL
    url += eventName;
    url += "/with/key/";
    url += key;
    url += "?value1=";
    url += String(dis);

    Serial.print("Requesting URL: ");
    Serial.println(url);

    client.print(String("GET ") + url + " HTTP/1.1\r\n" +
                 "Host: " + host + "\r\n" +
                 "Connection: close\r\n\r\n");

    // TODO: millis() is unsigned, so "timeout - millis() < 0" can never be
    // true and this timeout never fires. See "Known issues" in the README.
    int timeout = millis() + 5000;
    while (client.available() == 0) {
      if (timeout - millis() < 0) {
        Serial.println("Client Timeout !");
        client.stop();
        return;
      }
    }

    while (client.available()) {            // print the server response
      String line = client.readStringUntil('\r');
      Serial.print(line);
    }
  }
}

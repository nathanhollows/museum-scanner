#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define LED_PIN D1  // Pin where the LED is connected (D1 corresponds to GPIO5)

const char *ssid = "ESP8266_AP";  // SSID of the access point
const char *password = "12345678";  // Password for the access point

ESP8266WebServer server(80);  // Create a web server on port 80

void handleRoot() {
  server.send(200, "text/plain", "Hello! Send /blink to blink the LED.");
}

void handleBlink() {
  server.send(200, "text/plain", "LED blinked!");
  blinkLED();
}

void setup() {
  pinMode(LED_PIN, OUTPUT);

  Serial.begin(115200);
  Serial.println();
  Serial.print("Configuring access point...");

  // Set up the WiFi access point
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);

  // Set up the HTTP server
  server.on("/", handleRoot);
  server.on("/blink", handleBlink);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();  // Handle incoming client requests
}

void blinkLED() {
    for (int i = 0; i < 256; i++) {
      analogWrite(LED_PIN, 1 / (1 + exp(((i / 21) - 6) * -1)) * 255);
      delay(150 / 256);
    }
  delay(100);
    for (int i = 256; i >= 0; i--) {
      analogWrite(LED_PIN, 1 / (1 + exp(((i / 21) - 6) * -1)) * 255);
      delay(150 / 256);
    }
}

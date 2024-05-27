#include <Wire.h>
#include <Adafruit_PN532.h>
#include <SPI.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "config.h"

#define PN532_SCK  (D5)
#define PN532_MISO (D6)
#define PN532_MOSI (D7)
#define PN532_SS   (D2)
#define PN532_RST  (D4)
#define BLINK_PIN  (D1)

const char *ssid = "ESP8266_AP";  // SSID of the WiFi network
const char *password = "12345678";  // Password of the WiFi network
const char *serverIP = "192.168.4.1";  // IP address of the server
const int serverPort = 80;  // Port of the server

Adafruit_PN532 nfc(PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS);

void resetPN532() {
  digitalWrite(PN532_RST, LOW);
  delay(50);
  digitalWrite(PN532_RST, HIGH);
}

void blink() {
  digitalWrite(BLINK_PIN, HIGH);
  delay(50);
  digitalWrite(BLINK_PIN, LOW);
  delay(50);
  digitalWrite(BLINK_PIN, HIGH);
  delay(50);
  digitalWrite(BLINK_PIN, LOW);
  delay(50);
  digitalWrite(BLINK_PIN, HIGH);
  delay(50);
  digitalWrite(BLINK_PIN, LOW);
}

void sendBlinkRequest() {
  WiFiClient client;
  HTTPClient http;

  String url = String("http://") + serverIP + ":" + serverPort + "/blink";

  Serial.print("Sending request to: ");
  Serial.println(url);

  if (http.begin(client, url)) {
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.printf("HTTP GET request sent, response code: %d\n", httpCode);
      if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println("Response payload: " + payload);
      }
    } else {
      Serial.printf("HTTP GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("Unable to connect to server");
  }
}

void setup(void) {
  pinMode(PN532_RST, OUTPUT);
  pinMode(BLINK_PIN, OUTPUT);

  Serial.begin(9600);
  Serial.println("Hello!");

  // Connect to WiFi
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  resetPN532();

  while (true) {
    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();

    if (versiondata) {
      nfc.SAMConfig();
      Serial.println("Waiting for an NFC card ...");
      break;
    } else {
      Serial.println("Didn't find PN53x board, retrying...");
      resetPN532();
      delay(200);
    }
  }
}

void loop(void) {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };
  uint8_t uidLength;

  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    Serial.println("Found an NFC card!");
    Serial.print("UID Length: "); Serial.print(uidLength, DEC); Serial.println(" bytes");
    Serial.print("UID Value: ");
    for (uint8_t i = 0; i < uidLength; i++) {
      Serial.print(" 0x"); Serial.print(uid[i], HEX);
    }
    Serial.println("");

    blink();
    sendBlinkRequest();
    delay(500);

    resetPN532();
    while (true) {
      nfc.begin();
      uint32_t versiondata = nfc.getFirmwareVersion();
      if (versiondata) {
        nfc.SAMConfig();
        Serial.println("Ready for another scan.");
        break;
      } else {
        Serial.println("Didn't find PN53x board, retrying...");
        resetPN532();
        delay(200);
      }
    }
  }

  delay(10);
}

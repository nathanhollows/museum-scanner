#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define RST_PIN   D4     // RST-PIN for MFRC522
#define SS_PIN    D2     // SDA-PIN for MFRC522
#define BLINK_PIN LED_BUILTIN     // LED pin

const char *ssid = "ESP8266_AP";  // SSID of the WiFi network
const char *password = "12345678";  // Password of the WiFi network
const char *serverIP = "192.168.4.1";  // IP address of the server
const int serverPort = 80;  // Port of the server

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

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
  delay(50);
  digitalWrite(BLINK_PIN, HIGH);
}

void sendBlinkRequest(String boardID, String tagID) {
  WiFiClient client;
  HTTPClient http;

  String url = String("http://") + serverIP + ":" + serverPort + "/blink?boardID=" + boardID + "&tagID=" + tagID;

  Serial.print("Sending request to: ");
  Serial.println(url);

  if (http.begin(client, url)) {
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.printf("HTTP GET request sent, response code: %d\n", httpCode);
    } else {
      Serial.printf("HTTP GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
  } else {
    Serial.println("Unable to connect to server");
  }
}

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();
  int retryInterval = 500;  // Start with 500ms interval

  while (WiFi.status() != WL_CONNECTED) {
    delay(retryInterval);
    Serial.print(".");
    
    // Exponential backoff logic
    retryInterval *= 2;  // Double the interval for exponential backoff
    if (retryInterval > 32000) {
      retryInterval = 32000;  // Cap at 32 seconds
    }
    
    if (millis() - startAttemptTime > 60000) {
      // After 60 seconds of attempts, restart the device
      Serial.println("Restarting device due to prolonged connection attempts...");
      ESP.restart();
    }
  }
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup(void) {
  pinMode(BLINK_PIN, OUTPUT);
  digitalWrite(BLINK_PIN, HIGH);

  Serial.begin(9600);
  Serial.println("Hello!");

  connectToWiFi();  // Connect to WiFi

  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
  Serial.println("Scan PICC to see UID and type...");
}

void loop(void) {
  // Check WiFi connection status
  if (WiFi.status() != WL_CONNECTED) {
    connectToWiFi();
  }

  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  // Show UID on serial monitor
  Serial.print("Card UID:");
  String tagID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(" 0x");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    tagID += String(mfrc522.uid.uidByte[i], HEX);
  }
  Serial.println();

  blink();
  
  // Send board ID and tag ID
  String boardID = WiFi.macAddress();
  sendBlinkRequest(boardID, tagID);

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}

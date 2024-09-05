#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define RST_PIN   D4     // RST-PIN for MFRC522
#define SS_PIN    D2     // SDA-PIN for MFRC522
#define BLINK_PIN D3     // LED pin

const char *ssid = "";  // SSID of the WiFi network
const char *password = "";  // Password of the WiFi network
const char *server = "";  // Address of the server (no trailing slash)
const char *location = ""; // Unique location ID (must match content file but without `.md`)

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

// fadeLED functions
//
// These functions blink the LED using an S-curve function
// which is a sigmoid function that maps the input value  
// to a value between 0 and 255.
//
// This approach is useful as human eyes perceive light
// intensity in a non-linear way, and the S-curve function
// provides a more natural-looking blink.
//
// A too-high duration value will cause the blink to be
// jittery.
//
// The formula for the S-curve function is:
// =1/(1+EXP(((A2/21)-6)*-1))*255
// https://electronics.stackexchange.com/a/11100
// This function blinks the LED off using an S-curve function
void fadeLED(int duration, int startValue, int endValue) {
  if (startValue < endValue) {
    for (int i = startValue; i < endValue; i++) {
      analogWrite(BLINK_PIN, 1 / (1 + exp(((i / 21) - 6) * -1)) * 255);
      delay(duration / 256);
    }
  } else {
    for (int i = startValue; i >= endValue; i--) {
      analogWrite(BLINK_PIN, 1 / (1 + exp(((i / 21) - 6) * -1)) * 255);
      delay(duration / 256);
    }
  }
}

void connectToWiFi() {
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  unsigned long startAttemptTime = millis();
  int retryInterval = 500;  // Start with 500ms interval

  while (WiFi.status() != WL_CONNECTED) {
    fadeLED(250, 0, 256); 
    fadeLED(250, 256, 0); 
    delay(retryInterval);
    Serial.print(".");
    
    // Exponential backoff logic
    retryInterval *= 2;  // Double the interval for exponential backoff
    if (retryInterval > 32000) {
      retryInterval = 32000;  // Cap at 32 seconds
    }
    
    if (millis() - startAttemptTime > 60000) {
      digitalWrite(BLINK_PIN, LOW);
      // After 60 seconds of attempts, restart the device
      Serial.println("Restarting device due to prolonged connection attempts...");
      ESP.restart();
    }
  }
  digitalWrite(BLINK_PIN, HIGH);
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void sendBlinkRequest(String boardID, String tagID) {
  WiFiClient client;
  HTTPClient http;

  String url = String(server) + "/scan?location=" + String(location) + "&tag=" + tagID;

  Serial.print("Sending request to: ");
  Serial.println(url);

  if (http.begin(client, url)) {
    fadeLED(250, 256, 0); 
    fadeLED(100, 0, 256); 
    fadeLED(100, 256, 0); 
    fadeLED(100, 0, 256); 
    fadeLED(200, 256, 0); 
    int httpCode = http.GET();

    if (httpCode > 0) {
      Serial.printf("HTTP GET request sent, response code: %d\n", httpCode);
    } else {
      Serial.printf("HTTP GET request failed, error: %s\n", http.errorToString(httpCode).c_str());
    }
    http.end();
    fadeLED(400, 0, 256); 
  } else {
    Serial.println("Unable to connect to server");
  }
}

void setup(void) {
  pinMode(BLINK_PIN, OUTPUT);
  digitalWrite(BLINK_PIN, LOW);

  Serial.begin(9600);
  Serial.println("Hello!");

  fadeLED(250, 0, 256); 
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

  // Send board ID and tag ID
  String boardID = WiFi.macAddress();
  sendBlinkRequest(boardID, tagID);

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}

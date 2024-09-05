#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#define RST_PIN   D4     // RST-PIN for MFRC522
#define SS_PIN    D2     // SDA-PIN for MFRC522
#define BLINK_PIN LED_BUILTIN     // LED pin

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

void writeIDToTag(String id) {
  byte block4[16] = {0};
  byte block5[16] = {0};
  
  // Copy the ID to the byte arrays, splitting between block4 and block5
  for (int i = 0; i < 4; i++) {
    block4[i] = id[i];
  }
  for (int i = 4; i < 6; i++) {
    block5[i - 4] = id[i];
  }

  // Write to block 4
  MFRC522::StatusCode status = mfrc522.MIFARE_Write(4, block4, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed (block 4): "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  } else {
    Serial.println(F("Block 4 written successfully"));
  }

  // Write to block 5
  status = mfrc522.MIFARE_Write(5, block5, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() failed (block 5): "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  } else {
    Serial.println(F("Block 5 written successfully"));
  }
}

void writeURLToTag(String id) {
  String url = "https://example.com/trip/" + id;
  byte urlLength = url.length();
  byte urlBytes[urlLength + 1];
  urlBytes[0] = 0x03; // Prefix for "https://"

  // Copy the URL to the byte array
  for (int i = 0; i < urlLength; i++) {
    urlBytes[i + 1] = url[i];
  }

  // Create the NDEF message
  byte ndefMessage[100] = {0};
  ndefMessage[0] = 0xD1; // NDEF record header
  ndefMessage[1] = 0x01; // Type length
  ndefMessage[2] = urlLength + 1; // Payload length
  ndefMessage[3] = 0x55; // Type name format
  for (int i = 0; i < urlLength + 1; i++) {
    ndefMessage[4 + i] = urlBytes[i];
  }

  // Write the NDEF message to the tag starting from block 0x0A
  byte block[16] = {0};
  int bytesWritten = 0;
  int currentBlock = 0x04;
  while (bytesWritten < (urlLength + 4)) {
    for (int i = 0; i < 4 && bytesWritten < (urlLength + 4); i++) {
      block[i] = ndefMessage[bytesWritten++];
    }

    MFRC522::StatusCode status = mfrc522.MIFARE_Write(currentBlock++, block, 16);
    if (status != MFRC522::STATUS_OK) {
      Serial.print(F("MIFARE_Write() failed (block "));
      Serial.print(currentBlock - 1);
      Serial.print(F("): "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    } else {
      Serial.print(F("Block "));
      Serial.print(currentBlock - 1);
      Serial.println(F(" written successfully"));
    }

    // Clear block for next iteration
    memset(block, 0, 16);
  }
}

void setup(void) {
  pinMode(BLINK_PIN, OUTPUT);
  digitalWrite(BLINK_PIN, HIGH);

  Serial.begin(9600);
  Serial.println("Hello!");

  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522
  Serial.println("Scan PICC to see UID and type...");
}

void loop(void) {
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

  // Write custom ID to the tag
  String customID = "123456"; // Replace with your 6-digit ID
  // writeIDToTag(customID);

  // Write URL to the tag
  writeURLToTag(customID);

  blink();
  
  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}

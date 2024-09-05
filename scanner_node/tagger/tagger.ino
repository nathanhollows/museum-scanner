#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN   D4     // RST-PIN for MFRC522
#define SS_PIN    D2     // SDA-PIN for MFRC522

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

String lastTagID = "";
unsigned long lastScanTime = 0;
const unsigned long debounceDuration = 500;  // 0.5 seconds

void setup(void) {
  Serial.begin(9600);

  SPI.begin();           // Init SPI bus
  mfrc522.PCD_Init();    // Init MFRC522

  // Indicate that the hardware is ready
  Serial.println("READY");
}

void loop(void) {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(50);
    return;
  }

  // Show UID on serial monitor
  String tagID = "";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    tagID += String(mfrc522.uid.uidByte[i], HEX);
  }

  unsigned long currentTime = millis();

  // Check if the tag ID is the same as last one and within the debounce period
  if (tagID == lastTagID && (currentTime - lastScanTime) < debounceDuration) {
    // Ignore this scan as it's considered a duplicate
    mfrc522.PICC_HaltA();
    mfrc522.PCD_StopCrypto1();
    return;
  }

  // Send tag ID over Serial
  Serial.println(tagID);

  // Update last seen tag ID and time
  lastTagID = tagID;
  lastScanTime = currentTime;

  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
}

#include <Arduino.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h> // Include the WiFi library
#include <WiFiClient.h>
#include <ArduinoHttpClient.h>
#include <Arduino_JSON.h>

#define SS_PIN  5  // ESP32 pin GPIO5 
#define RST_PIN 27 // ESP32 pin GPIO27 
#define WIFI_SSID "KARAN"
#define WIFI_PASSWORD "12345679"
#define SERVER_IP "192.168.20.131" // Change this to your server IP address
#define SERVER_PORT 3000 // Change this to your server port

MFRC522 rfid(SS_PIN, RST_PIN);
WiFiClient wifiClient;
HttpClient httpClient = HttpClient(wifiClient, SERVER_IP, SERVER_PORT);

void setup() {
  Serial.begin(9600);
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522
  
  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
}

void sendUIDToServer(String uid) {
  Serial.print("Sending UID to server: ");
  Serial.println(uid);

  // Construct the request URL
  String url = "/scan";
  String requestBody = "{\"uid\":\"" + uid + "\"}";


  httpClient.post(url, "application/json", requestBody);
  String response = httpClient.responseBody();
  JSONVar myObject = JSON.parse(response);
  if (JSON.typeof(myObject) == "undefined")
  {
    Serial.println("Parsing input failed!");
    return;
  }
  else
  {
    Serial.println("Parsing input success!");
    Serial.println(myObject);
  }
}


void loop() {
  if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been read
      String uidHex = ""; // String to store UID hex
      for (int i = 0; i < rfid.uid.size; i++) {
        uidHex += (rfid.uid.uidByte[i] < 0x10 ? "0" : ""); // add leading zero if needed
        uidHex += String(rfid.uid.uidByte[i], HEX); // convert byte to hex string
      }

      // Send UID hex to server
      sendUIDToServer(uidHex);

      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }
  }
  delay(1000); // Wait for 1 second before next scan
}


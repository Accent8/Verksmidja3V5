#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#define SDA 14  //Define SDA pins
#define SCL 13  //Define SCL pins
#include <Arduino.h>
#include <Keypad.h>
#define KEYPAD_ROWS 4
#define KEYPAD_COLS 3
#include <WiFi.h>
#include <WebServer.h>

/* Put your SSID & Password */
const char* ssid = "ESP32";         // Enter SSID here
const char* password = "12345678";  //Enter Password here

/* Put IP Address details */
IPAddress local_ip(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Keypad takkar
char keypadKeys[KEYPAD_ROWS][KEYPAD_COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};
byte keypadRowPins[KEYPAD_ROWS] = { 21, 12, 22, 27 };
byte keypadColPins[KEYPAD_COLS] = { 26, 25, 33 };
Keypad keypad(makeKeymap(keypadKeys), keypadRowPins, keypadColPins, KEYPAD_ROWS, KEYPAD_COLS);
int LED = 2;
int redLED = 4;
int speakerPin = 32;
unsigned long lastBeepTime = 0;
LiquidCrystal_I2C lcd(0x27, 16, 2);
int myArray[10];  // Sample array
int arraySize = sizeof(myArray) / sizeof(myArray[0]);
int arrayIndex = 0;
unsigned long countdownStartTime;
const int countdownDuration = 20 * 1000;
bool countdownActive = false;
bool redLEDState = false;

WebServer server(80);

void handle_OnConnect() {
  server.send(200, "text/html", SendHTML());
}

void handle_kveikja() {
  // Keyrir þegar takki er ýttur á vefsíðu
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time left:   s");
  lcd.setCursor(0, 1);
  countdownStartTime = millis();
  countdownActive = true;
  server.send(200, "text/html", SendHTML());
}


void handle_NotFound() {
  server.send(404, "text/plain", "Not found");
}

String SendHTML() {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>LED Control</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #3498db;}\n";
  ptr += ".button-on:active {background-color: #2980b9;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>Csgo bomba</h1>\n";

  ptr += "<a class=\"button button-off\" href=\"/kveikja\">Kveikja</a>\n";

  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}


void setup() {
  Serial.begin(9600);
  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(100);

  server.on("/", handle_OnConnect);
  server.on("/kveikja", handle_kveikja);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP server started");
  Wire.begin(SDA, SCL);
  if (!i2CAddrTest(0x27)) {
    lcd = LiquidCrystal_I2C(0x3F, 16, 2);
  }
  lcd.init();            // LCD driver initialization
  lcd.backlight();       // Open the backlight
  lcd.setCursor(0, 0);   // Move the cursor to row 0, column 0
  lcd.print("Standby");  // The print content is displayed on the LCD

  start();
}
void start() {

  pinMode(LED, OUTPUT);
  pinMode(redLED, OUTPUT);
  pinMode(speakerPin, OUTPUT);
  int keyArray[7];
  int arrayIndex = 0;
}
void loop() {
  server.handleClient();

  if (countdownActive) {
    unsigned long currentTime = millis();
    // Toggle the red LED every other second
    if (currentTime - lastBeepTime >= 1000) {
      tone(speakerPin, 1000, 100);

      if (redLEDState) {
        digitalWrite(redLED, LOW);
      } else {
        digitalWrite(redLED, HIGH);
      }

      redLEDState = !redLEDState;  // Toggle the LED state
      lastBeepTime = currentTime;
    }

    unsigned long elapsedTime = currentTime - countdownStartTime;
    unsigned long remainingTime = (countdownDuration > elapsedTime) ? (countdownDuration - elapsedTime) : 0;

    updateLCD(remainingTime);

    if (remainingTime == 0) {
      // Display "BOOM!" when countdown reaches 0
      lcd.clear();
      lcd.setCursor(5, 0);
      lcd.print("BOOM!");
      digitalWrite(redLED, HIGH);
      tone(speakerPin, 500, 1000);
      countdownActive = false;
    }
  }

  lcd.setCursor(0, 1);
  char key = keypad.getKey();

  if (key) {
    // Ef takki er ýttur á keypad

    tone(speakerPin, 2000, 50);
    Serial.println(key);

    if (countdownActive) {
      if (isdigit(key)) {
        int value = key - '0';
        if (arrayIndex < sizeof(myArray) / sizeof(myArray[0])) {
          myArray[arrayIndex] = value;
          arrayIndex++;
        } else {
          Serial.println("Array is full!");
        }
      } else if (key == '#') {
        if (arrayIndex == 3 && myArray[0] == 1 && myArray[1] == 2 && myArray[2] == 3) {
          // Ef réttur kóði er settur inn (123)

          Serial.println("Correct code!");
          digitalWrite(LED, HIGH);
          tone(speakerPin, 1500, 1000);
          lcd.clear();
          lcd.setCursor(5, 0);
          lcd.print("DEFUSED!");
          digitalWrite(redLED, LOW);
          countdownActive = false;
        } else {
          Serial.println("Incorrect code!");
          tone(speakerPin, 500, 300);
        }

        memset(myArray, 0, sizeof(myArray));
        arrayIndex = 0;

        // Clear the LCD display for array values
        lcd.setCursor(0, 1);
        lcd.print("                ");  // Clear the line
      }

    } else {
      // If countdown is not active, reset everything on '#' key press
      if (key == '#') {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Time left:   s");
        lcd.setCursor(0, 1);
        digitalWrite(LED, LOW);
        countdownActive = true;
        countdownStartTime = millis();
      }
    }
  }
}

bool i2CAddrTest(uint8_t addr) {
  Wire.begin();
  Wire.beginTransmission(addr);
  if (Wire.endTransmission() == 0) {
    return true;
  }
  return false;
}

void updateLCD(unsigned long remainingTime) {
  lcd.setCursor(11, 0);

  if (remainingTime / 1000 < 10) {
    lcd.print("0");
  }

  lcd.print(remainingTime / 1000);  // Display remaining seconds

  lcd.setCursor(0, 1);

  for (int i = 0; i < arrayIndex; i++) {
    lcd.print(myArray[i]);
    lcd.print(" ");
  }
}

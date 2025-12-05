#include <Arduino.h>
#include <Keypad.h>
#include <ESP32Servo.h>

// CONFIGURAZIONE PIN 
const int PIN_LED_ROSSO = 25;
const int PIN_LED_VERDE = 26;
const int PIN_SERVO     = 13;

const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Pin del keypad
byte rowPins[ROWS] = {18, 19, 21, 15};  // R1, R2, R3, R4
byte colPins[COLS] = {2, 4, 16, 17};    // C1, C2, C3, C4

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
Servo myServo;


String codiceSegreto = "";
String inputCorrente = "";
bool inFaseDiSetup = true;


void chiudiCabina() {
  myServo.write(0);
  digitalWrite(PIN_LED_ROSSO, HIGH);
  digitalWrite(PIN_LED_VERDE, LOW);
}

void apriCabina() {
  myServo.write(90);
  digitalWrite(PIN_LED_ROSSO, LOW);
  digitalWrite(PIN_LED_VERDE, HIGH);
}

void feedbackPositivo() {
  digitalWrite(PIN_LED_VERDE, LOW); delay(100);
  digitalWrite(PIN_LED_VERDE, HIGH); delay(100);
  digitalWrite(PIN_LED_VERDE, LOW); delay(100);
  digitalWrite(PIN_LED_VERDE, HIGH);
}

void feedbackNegativo() {
  digitalWrite(PIN_LED_ROSSO, LOW); delay(100);
  digitalWrite(PIN_LED_ROSSO, HIGH); delay(100);
  digitalWrite(PIN_LED_ROSSO, LOW); delay(100);
  digitalWrite(PIN_LED_ROSSO, HIGH);
}


// SETUP
void setup() {
  Serial.begin(115200);
  Serial.println("--- AVVIO CABINA ---");

  pinMode(PIN_LED_ROSSO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);

  myServo.setPeriodHertz(50);
  myServo.attach(PIN_SERVO, 500, 2400);

  chiudiCabina();

  digitalWrite(PIN_LED_VERDE, HIGH);
  delay(500);
  digitalWrite(PIN_LED_VERDE, LOW);

  Serial.println("INSERISCI NUOVO CODICE (4 cifre) E PREMI #");
}

// LOOP PRINCIPALE
void loop() {
  char key = keypad.getKey();

  if (key) {
    Serial.print("Tasto: ");
    Serial.println(key);

    // -------- FASE 1: CREAZIONE PASSWORD --------
    if (inFaseDiSetup) {
      if (key == '#') {
        if (inputCorrente.length() == 4) {
          codiceSegreto = inputCorrente;
          inFaseDiSetup = false;
          inputCorrente = "";
          Serial.println("PASSWORD SALVATA!");
          feedbackPositivo();
          chiudiCabina();
        } else {
          Serial.println("ERRORE: Servono 4 numeri.");
          inputCorrente = "";
          feedbackNegativo();
        }
      }
      else if (key == '*') {
        inputCorrente = "";
        Serial.println("Reset.");
        feedbackNegativo();
      }
      else if (key >= '0' && key <= '9') {
        if (inputCorrente.length() < 4) {
          inputCorrente += key;
        }
      }
    }

    // -------- FASE 2: APERTURA/CHIUSURA --------
    else {
      if (key == '*') {
        chiudiCabina();
        inputCorrente = "";
        Serial.println("CABINA CHIUSA!");
      }
      else if (key == '#') {
        inputCorrente = "";
        Serial.println("Input resettato");
      }
      else if (key >= '0' && key <= '9') {
        inputCorrente += key;
        Serial.println(inputCorrente);

        if (inputCorrente.length() == 4) {
          if (inputCorrente == codiceSegreto) {
            apriCabina();
            Serial.println("CABINA APERTA!");
            inputCorrente = "";
          } else {
            Serial.println("PASSWORD ERRATA");
            feedbackNegativo();
            inputCorrente = "";
          }
        }
      }
    }
  }
}

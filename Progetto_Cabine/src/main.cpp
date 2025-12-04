#include <Arduino.h>
#include <Keypad.h>
#include <ESP32Servo.h>

// DICHIARAZIONE PIN  
const int PIN_LED_ROSSO = 23;
const int PIN_LED_VERDE = 25;
const int PIN_SERVO = 13;

const byte ROWS = 4; 
const byte COLS = 4; 

char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// PIN COLLEGATI 
byte rowPins[ROWS] = {4, 14, 27, 26}; 
byte colPins[COLS] = {18, 19, 21, 22}; 

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);
Servo myServo;

// VARIABILI
String codiceSegreto = "";
String inputCorrente = "";
bool inFaseDiSetup = true;

// FUNZIONI
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

// Lampeggio veloce Verde (Conferma)
void feedbackPositivo() {
  digitalWrite(PIN_LED_VERDE, LOW); delay(100);
  digitalWrite(PIN_LED_VERDE, HIGH); delay(100);
  digitalWrite(PIN_LED_VERDE, LOW); delay(100);
  digitalWrite(PIN_LED_VERDE, HIGH); 
}

// Lampeggio veloce Rosso (Errore)
void feedbackNegativo() {
  digitalWrite(PIN_LED_ROSSO, LOW); delay(100);
  digitalWrite(PIN_LED_ROSSO, HIGH); delay(100);
  digitalWrite(PIN_LED_ROSSO, LOW); delay(100);
  digitalWrite(PIN_LED_ROSSO, HIGH);
}

void setup() {
  Serial.begin(115200);
  Serial.println("--- AVVIO CABINA ---");

  pinMode(PIN_LED_ROSSO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);

  myServo.setPeriodHertz(50); 
  myServo.attach(PIN_SERVO, 500, 2400);

  // Stato iniziale
  chiudiCabina();
  
  // Segnale di vita all'avvio: Tutti i led lampeggiano una volta
  digitalWrite(PIN_LED_VERDE, HIGH); 
  delay(500); 
  digitalWrite(PIN_LED_VERDE, LOW);
  Serial.println("INSERISCI NUOVO CODICE (4 cifre) E PREMI #");
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    Serial.print("Tasto: "); Serial.println(key); // Debug per terminale

    // --- FASE 1: CREAZIONE PASSWORD ---
    if (inFaseDiSetup) {
      if (key == '#') {
        if (inputCorrente.length() == 4) {
          codiceSegreto = inputCorrente;
          inFaseDiSetup = false;
          inputCorrente = "";
          Serial.println("PASSWORD SALVATA!");
          feedbackPositivo(); // Lampeggio verde
          chiudiCabina();     // Torna rosso fisso e servo chiuso
        } else {
          Serial.println("ERRORE: Servono 4 numeri.");
          inputCorrente = "";
          feedbackNegativo();
        }
      } else if (key == '*') {
        inputCorrente = "";
        Serial.println("Reset.");
        feedbackNegativo();
      } else {
        // Accetta solo numeri
        if (key >= '0' && key <= '9') {
          if (inputCorrente.length() < 4) {
            inputCorrente += key;
          }
        }
      }
    } 
    
    // --- FASE 2: APERTURA/CHIUSURA ---
    else {
      // Tasto CHIUDI IMMEDIATAMENTE
      if (key == '*') {
        chiudiCabina();
        inputCorrente = "";
        Serial.println("CHIUSURA FORZATA.");
      } 
      // Tasto RESET INSERIMENTO
      else if (key == '#') {
        inputCorrente = "";
        Serial.println("Input resettato.");
      } 
      // INSERIMENTO NUMERI
      else {
        if (key >= '0' && key <= '9') {
            inputCorrente += key;
            Serial.println(inputCorrente);
            
            // Check immediato alla 4a cifra
            if (inputCorrente.length() == 4) {
              if (inputCorrente == codiceSegreto) {
                apriCabina();
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
}
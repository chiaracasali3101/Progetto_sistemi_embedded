#include <Keypad.h>
#include <Servo.h>

// ------------------------
// CONFIGURAZIONE KEYPAD
// ------------------------
const byte ROWS = 4;
const byte COLS = 4;

// Mappa dei tasti sul keypad
char hexaKeys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};

// Pin collegati alle righe (R1–R4)
byte rowPins[ROWS] = {5, 4, 3, 2};

// Pin collegati alle colonne (C1–C4)
byte colPins[COLS] = {A3, A2, A1, A0};

Keypad keypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);

// ------------------------
// SERVO & LED
// ------------------------
Servo lockServo;
const int SERVO_PIN = 6;

// Posizioni del servo (puoi modificarle se serve)
const int SERVO_LOCK_POS   = 0;   // cabina bloccata
const int SERVO_UNLOCK_POS = 90;  // cabina sbloccata

const int LED_ROSSO = 13; // led1 tramite r2 (bloccata)
const int LED_VERDE = 12; // led2 tramite r1 (sbloccata)

// ------------------------
// CODICE SEGRETO
// ------------------------
const byte CODE_LENGTH = 4;

// Codice segreto scelto dall'utente all'avvio
char secretCode[CODE_LENGTH + 1] = "0000";

// Buffer per il codice che l'utente sta inserendo
char enteredCode[CODE_LENGTH + 1];
byte codeIndex = 0;

// Stato cabina
bool isUnlocked = false;

// Stato: il codice è già stato impostato?
bool isCodeSet = false;

// ------------------------
// FUNZIONI DI STATO
// ------------------------
void lockCabin() {
  isUnlocked = false;
  digitalWrite(LED_ROSSO, HIGH); // rosso acceso
  digitalWrite(LED_VERDE, LOW);  // verde spento
  lockServo.write(SERVO_LOCK_POS);
}

void unlockCabin() {
  isUnlocked = true;
  digitalWrite(LED_ROSSO, LOW);  // rosso spento
  digitalWrite(LED_VERDE, HIGH); // verde acceso
  lockServo.write(SERVO_UNLOCK_POS);
}

void clearEnteredCode() {
  codeIndex = 0;
  for (byte i = 0; i < CODE_LENGTH; i++) {
    enteredCode[i] = '\0';
  }
}

bool checkCode() {
  enteredCode[CODE_LENGTH] = '\0'; // terminatore stringa
  for (byte i = 0; i < CODE_LENGTH; i++) {
    if (enteredCode[i] != secretCode[i]) {
      return false;
    }
  }
  return true;
}

// Copia enteredCode dentro secretCode
void saveNewCode() {
  for (byte i = 0; i < CODE_LENGTH; i++) {
    secretCode[i] = enteredCode[i];
  }
  secretCode[CODE_LENGTH] = '\0';
}

// ------------------------
// SETUP
// ------------------------
void setup() {
  Serial.begin(9600);

  pinMode(LED_ROSSO, OUTPUT);
  pinMode(LED_VERDE, OUTPUT);

  lockServo.attach(SERVO_PIN);

  // Stato iniziale: cabina bloccata
  lockCabin();
  clearEnteredCode();

  Serial.println("=== IMPOSTAZIONE CODICE INIZIALE ===");
  Serial.println("Digita un codice di 4 cifre e premi # per confermare");
  Serial.println("(Durante questa fase * cancella il codice inserito)");
}

// ------------------------
// LOOP PRINCIPALE
// ------------------------
void loop() {
  char key = keypad.getKey();

  if (!key) {
    return; // nessun tasto premuto
  }

  Serial.print("Tasto premuto: ");
  Serial.println(key);

  // ------------------------
  // FASE 1: IMPOSTAZIONE CODICE (all'avvio)
  // ------------------------
  if (!isCodeSet) {
    // Durante la fase di setup:
    // * -> cancella il buffer
    if (key == '*') {
      clearEnteredCode();
      Serial.println("Codice cancellato (*) in fase di setup");
      return;
    }

    // # -> conferma il codice se lungo CODE_LENGTH
    if (key == '#') {
      if (codeIndex == CODE_LENGTH) {
        saveNewCode();
        isCodeSet = true;
        Serial.print("Codice impostato: ");
        Serial.println(secretCode);

        // Segnale visivo: lampeggio LED verde
        for (int i = 0; i < 3; i++) {
          digitalWrite(LED_VERDE, HIGH);
          delay(150);
          digitalWrite(LED_VERDE, LOW);
          delay(150);
        }

        // Alla fine, cabina bloccata di default
        lockCabin();
        clearEnteredCode();
        Serial.println("Setup completato. Usa il codice per sbloccare la cabina.");
      } else {
        Serial.println("ERRORE: il codice deve avere 4 cifre prima di premere #");
        // lampeggia il rosso per indicare errore
        for (int i = 0; i < 3; i++) {
          digitalWrite(LED_ROSSO, LOW);
          delay(150);
          digitalWrite(LED_ROSSO, HIGH);
          delay(150);
        }
        clearEnteredCode();
      }
      return;
    }

    // Accettiamo solo numeri per il codice
    if (key >= '0' && key <= '9') {
      if (codeIndex < CODE_LENGTH) {
        enteredCode[codeIndex] = key;
        codeIndex++;
        Serial.print("Inserimento codice (setup): ");
        Serial.println(enteredCode);
      } else {
        // Se per qualche motivo si sfora la lunghezza, resettiamo
        Serial.println("Troppi caratteri inseriti, ricomincia il codice.");
        clearEnteredCode();
      }
    }

    // Ignoriamo A, B, C, D in setup
    return;
  }

  // ------------------------
  // FASE 2: FUNZIONAMENTO NORMALE (codice già impostato)
  // ------------------------

  // Se viene premuto '*', blocco immediatamente la cabina
  if (key == '*') {
    lockCabin();
    clearEnteredCode();
    Serial.println("Cabina BLOCCATA da tastierino (*)");
    return;
  }

  // Se viene premuto '#', cancella il codice inserito finora
  if (key == '#') {
    clearEnteredCode();
    Serial.println("Codice cancellato (#)");
    return;
  }

  // Accettiamo solo tasti numerici per il codice
  if (key >= '0' && key <= '9') {
    if (codeIndex < CODE_LENGTH) {
      enteredCode[codeIndex] = key;
      codeIndex++;
      Serial.print("Inserito: ");
      Serial.println(enteredCode);

      // Se abbiamo raggiunto CODE_LENGTH caratteri, controlliamo il codice
      if (codeIndex == CODE_LENGTH) {
        if (checkCode()) {
          Serial.println("CODICE CORRETTO -> cabina SBLOCCATA");
          unlockCabin();
        } else {
          Serial.println("Codice ERRATO -> cabina resta bloccata");

          // lampeggia il LED rosso per segnalare errore
          for (int i = 0; i < 3; i++) {
            digitalWrite(LED_ROSSO, LOW);
            delay(150);
            digitalWrite(LED_ROSSO, HIGH);
            delay(150);
          }
          // Assicuriamoci che resti nello stato "bloccato"
          lockCabin();
        }

        // Dopo aver controllato il codice, resettiamo il buffer
        clearEnteredCode();
      }
    } else {
      // Nel dubbio, resettiamo
      clearEnteredCode();
    }
  }
}

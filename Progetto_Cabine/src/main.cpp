#include <Arduino.h>
#include <Keypad.h>
#include <ESP32Servo.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

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

// ================== WIFI (Wokwi) ==================
const char* ssid     = "Wokwi-GUEST";
const char* password = "";

// ================== MQTT (HiveMQ Cloud) ==================
const char* mqtt_broker = "9e575485d8be46f0b26fc67805967c19.s1.eu.hivemq.cloud";
const int   mqtt_port   = 8883;

// ⚠️ METTI QUI USERNAME E PASSWORD MQTT DI HIVEMQ CLOUD
const char* mqtt_user   = "ProgettoEmbedded";   
const char* mqtt_pass   = "Password1";    


WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

// ================== FUNZIONI CABINA ==================

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

// ================== SUPPORTO MQTT ==================

bool cabinaAperta() {
  return digitalRead(PIN_LED_VERDE) == HIGH;
}

String creaPayloadStato() {
  String json = "{";
  json += "\"inFaseDiSetup\":"; json += (inFaseDiSetup ? "true" : "false"); json += ",";
  json += "\"cabinaAperta\":";  json += (cabinaAperta() ? "true" : "false");
  json += "}";
  return json;
}

void mqttReconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Connessione a HiveMQ Cloud...");

    String clientId = "esp32-cabina-";
    clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

    if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println(" connesso!");
      // Se vuoi ricevere comandi:
      // mqttClient.subscribe("cabina/comandi");
    } else {
      Serial.print(" fallita, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" nuovo tentativo tra 5s");
      delay(5000);
    }
  }
}

// callback opzionale per messaggi in ingresso (se in futuro vuoi comandi)
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Messaggio arrivato su topic: ");
  Serial.println(topic);

  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }
  Serial.print("Payload: ");
  Serial.println(msg);

  // Esempio: gestire comandi su "cabina/comandi"
  // if (String(topic) == "cabina/comandi") { ... }
}

// ================== SETUP ==================

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

  // --- CONNESSIONE WIFI (Wokwi) ---
  WiFi.begin(ssid, password, 6); // canale 6 per Wokwi-GUEST

  Serial.print("Connessione a ");
  Serial.println(ssid);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connesso!");
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());

  // --- TLS + MQTT ---
  espClient.setInsecure();
  mqttClient.setServer(mqtt_broker, mqtt_port);
  mqttClient.setCallback(mqttCallback);

  mqttReconnect();
}

// ================== LOOP ==================

void loop() {
  if (!mqttClient.connected()) {
    mqttReconnect();
  }
  mqttClient.loop();

  // Pubblica lo stato ogni 1000 ms
  static unsigned long lastPublish = 0;
  unsigned long now = millis();
  if (now - lastPublish > 1000) {
    lastPublish = now;
    String payload = creaPayloadStato();
    Serial.print("Pubblico su cabina/stato: ");
    Serial.println(payload);
    mqttClient.publish("cabina/stato", payload.c_str());
  }

  // ----- LOGICA ORIGINALE DELLA CABINA -----
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

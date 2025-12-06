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

// USERNAME E PASSWORD MQTT DI HIVEMQ CLOUD
const char* mqtt_user   = "ProgettoEmbedded";   
const char* mqtt_pass   = "Password1";    

// Topic MQTT
const char* MQTT_TOPIC_STATO   = "cabina/stato";
const char* MQTT_TOPIC_COMANDI = "cabina/comandi";

WiFiClientSecure espClient;
PubSubClient mqttClient(espClient);

// Prototipo
void pubblicaStato(const char* motivo);

// ================== FUNZIONI CABINA ==================
void chiudiCabina() {
  myServo.write(0);
  digitalWrite(PIN_LED_ROSSO, HIGH);
  digitalWrite(PIN_LED_VERDE, LOW);
  pubblicaStato("chiudiCabina");
}

void apriCabina() {
  myServo.write(90);
  digitalWrite(PIN_LED_ROSSO, LOW);
  digitalWrite(PIN_LED_VERDE, HIGH);
  pubblicaStato("apriCabina");
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

void pubblicaStato(const char* motivo) {
  if (!mqttClient.connected()) {
    Serial.print("MQTT non connesso, impossibile pubblicare stato (");
    Serial.print(motivo);
    Serial.println(")");
    return;
  }
  String payload = creaPayloadStato();
  Serial.print("Pubblico stato (");
  Serial.print(motivo);
  Serial.print(") su ");
  Serial.print(MQTT_TOPIC_STATO);
  Serial.print(": ");
  Serial.println(payload);

  mqttClient.publish(MQTT_TOPIC_STATO, payload.c_str());
}

void mqttReconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Connessione a HiveMQ Cloud...");

    String clientId = "esp32-cabina-";
    clientId += String((uint32_t)ESP.getEfuseMac(), HEX);

    if (mqttClient.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println(" connesso!");

      // Sottoscrizione al topic comandi 
      if (mqttClient.subscribe(MQTT_TOPIC_COMANDI)) {
        Serial.print("Sottoscritto a ");
        Serial.println(MQTT_TOPIC_COMANDI);
      } else {
        Serial.println("Errore subscribe al topic comandi");
      }

      // Pubblico stato immediatamente dopo la riconnessione
      pubblicaStato("reconnect");

    } else {
      Serial.print(" fallita, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" nuovo tentativo tra 5s");
      delay(5000);
    }
  }
}

// callback per messaggi in ingresso 
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Messaggio arrivato su topic: ");
  Serial.println(topic);

  // ricostruisco il payload in una String leggibile
  String msg;
  for (unsigned int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("Payload: ");
  Serial.println(msg);

  // Se il messaggio arriva dal topic dei comandi
  if (String(topic) == MQTT_TOPIC_COMANDI) {
    if (msg == "APRI") {
      Serial.println("Comando remoto: APRI CABINA");
      apriCabina();
    } else if (msg == "CHIUDI") {
      Serial.println("Comando remoto: CHIUDI CABINA");
      chiudiCabina();
    } else {
      Serial.println("Comando remoto sconosciuto");
    }
  }
}

// ================== SETUP ==================
void setup() {
  Serial.begin(115200);
  Serial.println("--- AVVIO CABINA ---");

  pinMode(PIN_LED_ROSSO, OUTPUT);
  pinMode(PIN_LED_VERDE, OUTPUT);

  myServo.setPeriodHertz(50);
  myServo.attach(PIN_SERVO, 500, 2400);

  digitalWrite(PIN_LED_VERDE, HIGH);
  delay(500);
  digitalWrite(PIN_LED_VERDE, LOW);

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
  espClient.setInsecure();     // Disabilita la verifica del certificato (ok per test)
  mqttClient.setServer(mqtt_broker, mqtt_port);
  mqttClient.setCallback(mqttCallback);

  // Connessione al broker MQTT
  mqttReconnect();

  // imposto lo stato iniziale della cabina e lo pubblico
  chiudiCabina(); // inizio con cabina chiusa 

  Serial.println("INSERISCI NUOVO CODICE (4 cifre) E PREMI #");
}

// ================== LOOP ==================
void loop() {
  if (!mqttClient.connected()) {
    mqttReconnect();
  }
  mqttClient.loop();

  // Heartbeat: pubblica lo stato ogni 30 secondi (per sicurezza)
  static unsigned long lastPublish = 0;
  unsigned long now = millis();
  if (now - lastPublish > 30000) {  
    lastPublish = now;
    pubblicaStato("heartbeat");
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
          pubblicaStato("fine_setup");
        } else {
          Serial.println("ERRORE: Servono 4 numeri.");
          inputCorrente = "";
          feedbackNegativo();
          pubblicaStato("errore_setup");
        }
      }
      else if (key == '*') {
        inputCorrente = "";
        Serial.println("Reset.");
        feedbackNegativo();
        pubblicaStato("reset_setup");
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
        // chiudiCabina già chiama pubblicaStato()
      }
      else if (key == '#') {
        inputCorrente = "";
        Serial.println("Input resettato");
        pubblicaStato("reset_codice");
      }
      else if (key >= '0' && key <= '9') {
        inputCorrente += key;
        Serial.println(inputCorrente);

        if (inputCorrente.length() == 4) {
          if (inputCorrente == codiceSegreto) {
            apriCabina();
            Serial.println("CABINA APERTA!");
            inputCorrente = "";
            // apriCabina già chiama pubblicaStato()
          } else {
            Serial.println("PASSWORD ERRATA");
            feedbackNegativo();
            inputCorrente = "";
            pubblicaStato("password_errata");
          }
        }
      }
    }
  }
}

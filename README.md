# 📝**Sistema di Apertura Cabina con Tastierino, Servo ed Interfaccia IoT (ESP32 + MQTT)**

## Descrizione Generale

Il progetto realizza un **sistema di controllo accessi per una cabina** utilizzando:

* **ESP32 DevKit**
* **Tastierino 4×4**
* **Servo motore** per apertura/chiusura
* **LED rosso e verde** come indicatori di stato
* **Connessione Wi-Fi**
* **Cloud MQTT (HiveMQ Cloud)**
* **Dashboard Web** per monitoraggio e controllo remoto

La cabina può essere aperta:

* **localmente**, inserendo un codice numerico sul tastierino
* **da remoto**, tramite un sito web collegato al broker MQTT

La chiusura può avvenire localmente con `*` o da remoto tramite comando MQTT.

---

# **Flusso di Avvio del Sistema**

Quando l’ESP32 viene acceso:

1. Inizializza i pin digitali, il servo e il tastierino
2. Si connette alla rete **Wi-Fi (Wokwi-GUEST)**
3. Si connette al broker **HiveMQ Cloud** tramite MQTT over TLS
4. Esegue la **chiusura iniziale della cabina** e pubblica lo stato su MQTT
5. Mostra il messaggio:
   **"INSERISCI NUOVO CODICE (4 cifre) E PREMI #”**

---

# **Impostazione del codice segreto**

Alla prima accensione è necessario impostare un codice a 4 cifre.

Procedura:

1. L’utente inserisce **4 cifre**
2. Premendo **`#`**:

   * Se il codice è valido → salvato in `codiceSegreto`
   * LED verde lampeggia (feedback positivo)
   * Il sistema esce dalla fase di setup (`inFaseDiSetup = false`)
3. Se il codice è troppo corto → errore con LED rosso (feedback negativo)

Premendo `*` durante il setup, l’inserimento viene cancellato.

---


## Apertura cabina

1. L’utente inserisce le 4 cifre del codice
2. Se corrette:

   * Servo → **90°** → cabina aperta
   * LED verde acceso
   * Stato pubblicato su MQTT (`cabina/stato`)

## Codice errato

* LED rosso lampeggia
* Input resettato
* Evento pubblicato su MQTT

## Chiusura cabina (manuale)

Premendo:

* `*` → chiude la cabina
* LED rosso acceso
* Stato pubblicato su MQTT

Premendo `#` durante la modalità operativa viene solo resettato l’input.

---

# 🌐 **Funzionalità IoT tramite MQTT (HiveMQ Cloud)**

Il sistema utilizza un broker cloud:

```
HiveMQ Cloud – MQTT over TLS (porta 8883)
```

L’ESP32:

### Pubblica lo stato della cabina

Topic:

```
cabina/stato
```

Payload (JSON):

```json
{
  "inFaseDiSetup": false,
  "cabinaAperta": true
}
```

Invio dello stato:

* **immediato** quando cambia apertura/chiusura
* ogni **30 secondi** (heartbeat)
* dopo ogni riconnessione MQTT

### Riceve comandi remoti

Topic:

```
cabina/comandi
```

Messaggi supportati:

* `"APRI"` → servo a 90°, LED verde ON
* `"CHIUDI"` → servo a 0°, LED rosso ON

Questi comandi possono essere inviati da:

* Web Client di HiveMQ
* Sito web sviluppato per il progetto
* Qualsiasi client MQTT

---

# 🖥️ **Dashboard Web – Controllo e Monitoraggio Remoto**

È stato sviluppato un file HTML che permette:

## Monitorare lo stato della cabina in tempo reale

* Cabina aperta / chiusa
* Modalità setup / operativa
* Log degli eventi
* Stato connessione MQTT

## Controllare la cabina da remoto

Due pulsanti:

* **Apri cabina**
* **Chiudi cabina**

Questi pulsanti inviano comandi MQTT (`APRI` / `CHIUDI`) all’ESP32.

La dashboard utilizza:

* **mqtt.js** (comunicazione MQTT via WebSocket)
* Collegamento a HiveMQ Cloud via `wss://...:8884/mqtt`
* Logica reattiva aggiornata automaticamente quando arrivano messaggi MQTT


---

# 🔩 **Componenti Utilizzati (Hardware)**

* ESP32 DevKit C
* Tastierino 4×4
* Servo SG90
* LED rosso + resistenza 220 Ω
* LED verde + resistenza 220 Ω
* Breadboard e jumper

---

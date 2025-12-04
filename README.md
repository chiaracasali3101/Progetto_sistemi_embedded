# Progetto Sistemi Embedded e IoT – Sistema di Apertura Cabine con Tastierino e Servo (ESP32)

## Descrizione Generale

Questo progetto implementa un sistema elettronico per la gestione dell'apertura e chiusura di una cabina (o più cabine) utilizzando:

* **ESP32 DevKit**
* **Tastierino numerico 4x4 (Keypad)**
* **Servo motore** per il meccanismo di blocco/sblocco
* **LED** per indicazione dello stato (bloccato/sbloccato)

Il sistema permette all'utente di **impostare un codice personalizzato all'avvio** e di utilizzare quel codice per aprire la cabina.
In qualsiasi momento è possibile richiudere la cabina tramite il tasto `*` del tastierino.

Il progetto simula perfettamente il funzionamento di cabine numerate come quelle presenti in spiagge, stabilimenti balneari, depositi o magazzini.

---

## Funzionamento del Sistema

### Impostazione iniziale del codice

Quando l’ESP32 viene acceso:

* Il **LED rosso** si accende → la cabina è *in stato bloccato*.
* Il sistema entra in **modalità setup del codice**.
* L’utente deve inserire **4 cifre** tramite tastierino.
* Una volta inserite, premere **`#` per confermare**.

Se il codice ha esattamente 4 cifre:

* Il codice viene salvato come *codice segreto*
* Il LED verde lampeggia 3 volte → *conferma visiva*
* Il sistema passa alla modalità operativa normale

Se il codice è incompleto, il LED rosso lampeggia per segnalare l’errore.

---

## Modalità di utilizzo normale

Dopo aver impostato il codice, il sistema è pronto all'uso.

### Sblocco della cabina

Per sbloccare la cabina:

* Inserisci le **4 cifre del codice corretto**
* Se il codice è corretto:

  * Il **LED verde** si accende
  * Il **LED rosso** si spegne
  * Il **servo motore ruota** nella posizione di sblocco
  * La cabina è aperta

### Codice errato

Se il codice inserito è sbagliato:

* Il LED rosso lampeggia 3 volte
* La cabina rimane chiusa
* Il servo resta nella posizione di blocco

### Chiusura manuale della cabina

Premendo il tasto **`*`** in qualsiasi momento:

* Il sistema blocca la cabina
* Il LED rosso si accende
* Il LED verde si spegne
* Il servo torna nella posizione di chiusura

---

## Componenti Utilizzati

* **ESP32 DevKit / ESP32-WROOM**
* **Tastierino a membrana 4x4**
* **Servo motore** (tipo SG90 o simile)
* **LED rosso + resistenza 220 Ω**
* **LED verde + resistenza 220 Ω**
* **Cavi jumper**
* **Breadboard**

---

## Collegamenti Principali (Pinout con ESP32)

### Keypad 4x4

I pin possono variare, qui una configurazione consigliata:

* R1 → GPIO 32
* R2 → GPIO 33
* R3 → GPIO 25
* R4 → GPIO 26
* C1 → GPIO 27
* C2 → GPIO 14
* C3 → GPIO 12
* C4 → GPIO 13

### Servo

* Segnale → GPIO 18
* VCC → 5V
* GND → GND (comune con ESP32)

### LED

* LED Rosso → GPIO 23 (catodo verso GND tramite resistenza)
* LED Verde → GPIO 22 (catodo verso GND tramite resistenza)

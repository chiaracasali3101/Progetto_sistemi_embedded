# Progetto Sistemi Embedded e IoT – Sistema di Apertura Cabine con Tastierino e Servo

## Descrizione Generale

Questo progetto implementa un sistema elettronico per la gestione dell'apertura e chiusura di una cabina (o più cabine) utilizzando:

* **Arduino UNO**
* **Tastierino numerico 4x4 (Keypad)**
* **Servo motore** per il meccanismo di blocco/sblocco
* **LED** per indicazione dello stato (bloccato/sbloccato)

Il sistema permette all'utente di **impostare un codice personalizzato all'avvio** e di utilizzare quel codice per aprire la cabina. In qualsiasi momento è possibile richiudere la cabina tramite il tasto `*` del tastierino.

Il progetto simula perfettamente il funzionamento di cabine numerate come quelle presenti in spiagge, stabilimenti balneari, depositi o magazzini.

---

## Funzionamento del Sistema

### Impostazione iniziale del codice

Quando l'Arduino viene acceso:

* Il **LED rosso** si accende → la cabina è *in stato bloccato*.
* Il sistema entra in **modalità setup del codice**.
* L'utente deve inserire **4 cifre** tramite tastierino.
* Una volta inserite, premere **`#` per confermare**.

Se il codice ha esattamente 4 cifre:

* Il codice viene salvato come *codice segreto*
* Il LED verde lampeggia 3 volte → *conferma visiva*
* Il sistema passa alla modalità di funzionamento normale

Se il codice è incompleto, il LED rosso lampeggia per segnalare l’errore.

---

## Modalità di utilizzo normale

Dopo aver impostato il codice, il sistema è pronto all'uso.

### Sblocco della cabina

Per sbloccare la cabina:

* Inserisci le **4 cifre del codice corretto**
* Se il codice è giusto:

  * Il **LED verde** si accende
  * Il **LED rosso** si spegne
  * Il **servo motore ruota** nella posizione di sblocco
  * La cabina è aperta

### Codice errato

Se il codice inserito è sbagliato:

* Il LED rosso lampeggia 3 volte
* La cabina rimane bloccata
* Il servo rimane nella posizione di chiusura

### Chiusura manuale della cabina

Premendo il tasto **`*`** in qualsiasi momento:

* Il sistema blocca la cabina
* Il LED rosso si accende
* Il LED verde si spegne
* Il servo torna nella posizione di blocco

---

## Componenti Utilizzati

* **Arduino UNO R3**
* **Tastierino a membrana 4x4**
* **Servo motore** (tipo SG90 o simile)
* **LED rosso + resistenza 220 Ω**
* **LED verde + resistenza 220 Ω**
* **Cavi jumper**
* **Breadboard**

---

## Collegamenti Principali (Pinout)

### Keypad 4x4

* R1 → D5
* R2 → D4
* R3 → D3
* R4 → D2
* C1 → A3
* C2 → A2
* C3 → A1
* C4 → A0

### Servo

* PWM → D6
* VCC → 5V
* GND → GND

### LED

* LED Rosso → D13 (catodo a GND via resistenza)
* LED Verde → D12 (catodo a GND via resistenza)

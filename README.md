## Progetto Sistemi Embedded e IoT – Sistema di Apertura Cabina con Tastierino e Servo (ESP32)

## Descrizione Generale

Il progetto implementa un sistema elettronico per **bloccare e sbloccare una cabina** tramite:

* **ESP32 DevKit**
* **Tastierino 4x4**
* **Servo motore** per il meccanismo di apertura/chiusura
* **LED rosso e verde** per indicare lo stato della cabina

All’avvio l’utente deve **impostare un codice di 4 cifre**, che diventa la password di sblocco.
Durante l’uso normale, l’inserimento della password corretta apre la cabina; il tasto `*` la richiude in qualsiasi momento.

---

## Funzionamento del Sistema

### Impostazione iniziale del codice

Quando l’ESP32 si accende:

* Viene eseguita la **chiusura iniziale** della cabina (servo a 0°)
* Il **LED rosso si accende**
* Il **LED verde lampeggia 1 volta** come segnale di avvio
* Il sistema entra nella **fase di setup della password**

L’utente deve:

1. Inserire **4 cifre**
2. Premere **`#`** per confermare

Se il codice è valido:

* Viene salvato in memoria volatile (`codiceSegreto`)
* Il LED verde esegue un **lampeggio rapido di conferma** (feedback positivo)
* Il sistema passa alla modalità operativa

Se il codice è errato (meno di 4 cifre):

* Il LED rosso lampeggia (feedback negativo)
* L’inserimento viene resettato

L’utente può premere `*` per cancellare l’input.

---

## Modalità di utilizzo normale

### Sblocco della cabina

Per aprire la cabina:

1. Inserire le **4 cifre** del codice
2. Se corrette:

   * Il servo ruota a **90°** → cabina aperta
   * LED verde acceso
   * LED rosso spento

Se il codice è errato:

* Il LED rosso lampeggia 2 volte (feedback negativo)
* L’input viene resettato

### Chiusura manuale della cabina

Premendo **`*`** in qualsiasi momento:

* La cabina viene chiusa (servo a 0°)
* LED rosso acceso
* LED verde spento
* L’input corrente viene cancellato

Premendo **`#`** nella modalità operativa:

* L’input viene semplicemente resettato, senza effetti sulla cabina

---

## Componenti Utilizzati

* **ESP32 DevKit C**
* **Keypad 4×4**
* **Servo SG90 (o compatibile)**
* **LED rosso (con resistenza da 220 Ω)**
* **LED verde (con resistenza da 220 Ω)**
* Breadboard e cavi jumper

---
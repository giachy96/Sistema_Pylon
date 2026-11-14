/*
   TX_GIUDICE_Rev2 - Telecomando GIUDICE DI GARA (LoRa E22) - Sistema Pylon
   Rev2: BLOCCO IN GARA. Dopo il GO il telecomando non trasmette piu' nulla (tranne lo STOP) finche' non ha
   "sentito" arrivare tutti e 3 gli stringoni finali (5514/5524/5534; per un pilota assente
   il Teensy manda 6514/6524/6534 con i campi vuoti: vale uguale) dal tabellone alla centrale. Cosi' non
   puo' mai collidere con i risultati. Si sblocca anche con il proprio STOP, se sente un 6000 della centrale
   (anche incollato ad altri pacchetti) e, per sicurezza se il 6000 si perde, dopo TIMEOUT_GARA ms dal Go.
   Se sente un 3000 (Go dato da altro) si blocca anch'esso. I dati degli stringoni non vengono memorizzati.
   Il modulo E22 ora ascolta: indirizzo 0xFFFF (monitor), canale 5, stessi parametri radio.

   Manda alla CENTRALE gli stessi codici dei pulsanti:
     INDIETRO = "700"   AVANTI = "800"   SHOW = "2000"   GO = "3000"   STOP = "6000"
   La centrale resta comunque quella che decide (anti doppia pressione, blocco in gara). Niente ripetizioni automatiche: se il comando si perde si preme di nuovo
   (l'esito si vede dal tabellone: nomi nuovi, Show, Go, Stop).

   Collegamenti (Arduino UNO/Nano, come i telecomandi dei pali):
   E22 TX -> pin 2 (PullUp)      E22 RX -> pin 3 (PullUp + partitore)     M0/M1 -> GND     AUX non collegato
   Display OLED SH1106 I2C (SDA/SCL)      Buzzer -> pin 5      Batteria -> A0 (stesso partitore degli altri TX)
   Pulsanti verso GND (INPUT_PULLUP):  INDIETRO = pin 10, AVANTI = pin 9, SHOW = pin 7, GO = pin 6, STOP = pin 4

   Modulo E22 del telecomando: canale 5, stessi parametri radio del tabellone (velocita' in aria, potenza,
   chiave, modalita' trasmissione fissa); l'indirizzo e' FFFF (ADDH=FF, ADDL=FF: riceve tutto
   cio' che passa sul canale 5; l'invio in trasmissione fissa funziona comunque).
   Destinazione: indirizzo 1 (centrale), canale 5.
*/
#include "U8x8lib.h"
#include "Arduino.h"
#include "LoRa_E22.h"

U8X8_SH1106_128X64_NONAME_HW_I2C u8x8(/* reset=*/U8X8_PIN_NONE);
LoRa_E22 e22ttl(2, 3);  // Arduino RX --> e22 TX - Arduino TX --> e22 RX

// ---- configurazione ----
const byte Key = 0;   // ADDH destinazione
const byte Add = 1;   // ADDL destinazione = modulo E22 della centrale (il modulo del tabellone e' 0)
const byte Chan = 5;  // canale centrale/tabellone

const byte NUM_PULSANTI = 5;
//                                  INDIETRO AVANTI SHOW GO STOP
const byte pinPulsante[NUM_PULSANTI] = { 10, 9, 7, 6, 4 };
const char* const codice[NUM_PULSANTI] = { "700", "800", "2000", "3000", "6000" };
const char* const nomeCmd[NUM_PULSANTI] = { " INDIETRO", " AVANTI", " SHOW", " GO", " STOP" };
const byte IDX_STOP = 4;
const byte IDX_GO = 3;

const byte buzzer = 5;
const byte pinbatt = A0;

const unsigned long PAUSA_COMANDI = 1500;  // tra due comandi (esclusi gli Stop)
const unsigned long PAUSA_STOP = 500;      // tra due Stop: lo Stop e' sempre disponibile
const unsigned long DEBOUNCE = 30;
const unsigned long DURATA_SCHERMATA = 2500;  // quanto resta a video il comando inviato
const unsigned long INTERVALLO_BATT = 5000;   // aggiornamento tensione a schermo
const float SOGLIA_BATT_ALLARME = 3.2;
const float SOGLIA_BATT_RIENTRO = 3.4;

// ---- blocco in gara ----
const unsigned long TIMEOUT_GARA = 240000UL;  // sblocco di sicurezza dopo 4 minuti dal Go (Stop da centrale/PC perso)
unsigned long inizioGara = 0;
bool inGara = false;      // true dal Go fino ai 3 stringoni (o Stop)
byte stringoniVisti = 0;  // bit0 = 5514, bit1 = 5524, bit2 = 5534

// ---- stato ----
bool premuto[NUM_PULSANTI];
unsigned long ultimoCambio[NUM_PULSANTI];
unsigned long ultimoComando = 0UL - PAUSA_COMANDI;
unsigned long ultimoStop = 0UL - PAUSA_STOP;
unsigned long inizioSchermata = 0;
unsigned long ultimoAggBatt = 0;
bool schermataComando = false;
bool batteriaBassa = false;

float leggiBatteria() {
  float t = analogRead(pinbatt);
  return (t / 1023) * 4.35;
}

void aggiornaVoltaggio(float v) {
  char vout[8];
  dtostrf(v, 6, 2, vout);
  u8x8.setFont(u8x8_font_8x13B_1x2_r);
  u8x8.drawString(1, 5, vout);
  u8x8.drawString(7, 5, " V");
}

void schermataStandby() {
  u8x8.clear();
  u8x8.setFont(u8x8_font_8x13B_1x2_r);
  u8x8.drawString(0, 0, " GIUDICE GARA");
  u8x8.drawString(0, 2, " Batteria");
  aggiornaVoltaggio(leggiBatteria());
  schermataComando = false;
  ultimoAggBatt = millis();
}

void schermataInvio(byte i, bool ok) {
  u8x8.clear();
  u8x8.setFont(u8x8_font_8x13B_1x2_r);
  u8x8.drawString(0, 0, " COMANDO");
  u8x8.drawString(0, 3, nomeCmd[i]);
  u8x8.drawString(0, 6, ok ? " INVIATO" : " ERRORE INVIO");
  schermataComando = true;
  inizioSchermata = millis();
}

void schermataGara() {
  u8x8.clear();
  u8x8.setFont(u8x8_font_8x13B_1x2_r);
  u8x8.drawString(0, 0, " IN GARA");
  u8x8.drawString(0, 3, " solo STOP");
  char riga[12];
  byte n = (stringoniVisti & 1) + ((stringoniVisti >> 1) & 1) + ((stringoniVisti >> 2) & 1);
  snprintf(riga, sizeof riga, " Risult. %d/3", n);
  u8x8.drawString(0, 6, riga);
  schermataComando = false;
  ultimoAggBatt = millis();
}

void entraInGara() {
  inGara = true;
  inizioGara = millis();
  stringoniVisti = 0;
}

void esciDaGara(bool beep) {
  inGara = false;
  stringoniVisti = 0;
  if (beep) tone(buzzer, 1500, 150);
  schermataStandby();
}

void invia(byte i) {
  tone(buzzer, 1000, 100);
  ResponseStatus rs = e22ttl.sendFixedMessage(Key, Add, Chan, String(codice[i]));
  bool ok = (rs.code == E22_SUCCESS);
  unsigned long t = millis();
  ultimoComando = t;
  if (i == IDX_STOP) ultimoStop = t;
  if (i == IDX_STOP && ok) inGara = false, stringoniVisti = 0;       // lo Stop riabilita tutto
  else if (i == IDX_GO && ok) entraInGara();
  schermataInvio(i, ok);
  Serial.print("invio ");
  Serial.print(codice[i]);
  Serial.println(ok ? " ok" : " ERRORE");
}

void pressione(byte i) {
  unsigned long t = millis();
  if (inGara && i != IDX_STOP) {
    tone(buzzer, 500, 80);  // in gara: solo lo Stop
    return;
  }
  if (i == IDX_STOP) {
    if (t - ultimoStop < PAUSA_STOP) return;  // anti rimbalzo dello Stop, indipendente dagli altri tasti
  } else {
    if (t - ultimoComando < PAUSA_COMANDI) {
      tone(buzzer, 500, 80);  // comando ignorato: troppo ravvicinato
      return;
    }
  }
  invia(i);
}

// ascolto del canale: stringoni finali e codici della centrale
void ascolta() {
  if (e22ttl.available() > 1) {
    ResponseContainer rc = e22ttl.receiveMessage();
    if (rc.status.code != 1) return;  // 1 = E22_SUCCESS
    String m = rc.data;
    bool cambiato = false;
    if (m == "3000" && !inGara) {  // Go dato da un altro comando: ci si blocca
      entraInGara();
      cambiato = true;
    }
    if (inGara && m.indexOf(',') < 0 && m.indexOf("6000") >= 0) {  // Stop della centrale (anche incollato)
      esciDaGara(true);
      return;
    }
    if (inGara) {
      // per ogni colore: 55x4 = arrivato, 65x4 = timeout (pilota assente) -> in entrambi i casi e' il suo stringone
      if (m.indexOf("5514,") >= 0 || m.indexOf("6514,") >= 0) stringoniVisti |= 1;
      if (m.indexOf("5524,") >= 0 || m.indexOf("6524,") >= 0) stringoniVisti |= 2;
      if (m.indexOf("5534,") >= 0 || m.indexOf("6534,") >= 0) stringoniVisti |= 4;
      cambiato = true;
      if (stringoniVisti == 7) {
        esciDaGara(true);
        return;
      }
    }
    if (cambiato && inGara) schermataGara();
  }
}

void setup() {
  for (byte i = 0; i < NUM_PULSANTI; i++) {
    pinMode(pinPulsante[i], INPUT_PULLUP);
    premuto[i] = false;
    ultimoCambio[i] = 0;
  }
  pinMode(buzzer, OUTPUT);
  pinMode(pinbatt, INPUT);
  Serial.begin(9600);
  delay(500);
  e22ttl.begin();
  u8x8.begin();
  u8x8.setPowerSave(0);
  schermataStandby();
  tone(buzzer, 1000, 200);
}

void loop() {
  unsigned long now = millis();

  // pulsanti: si controlla prima lo STOP (indice piu' alto), un comando per pressione
  for (int i = NUM_PULSANTI - 1; i >= 0; i--) {
    bool basso = (digitalRead(pinPulsante[i]) == LOW);
    if (basso != premuto[i] && (now - ultimoCambio[i]) >= DEBOUNCE) {
      premuto[i] = basso;
      ultimoCambio[i] = now;
      if (basso) pressione(i);
    }
  }

  ascolta();
  if (inGara && (now - inizioGara >= TIMEOUT_GARA)) esciDaGara(true);  // sblocco di sicurezza
  if (inGara && !schermataComando && now - ultimoAggBatt >= INTERVALLO_BATT) schermataGara();

  // batteria: allarme con isteresi
  float v = leggiBatteria();
  if (v < SOGLIA_BATT_ALLARME) {
    batteriaBassa = true;
    digitalWrite(buzzer, HIGH);
  } else if (batteriaBassa && v > SOGLIA_BATT_RIENTRO) {
    batteriaBassa = false;
    digitalWrite(buzzer, LOW);
  }

  // display: dopo il comando torna alla schermata batteria; poi aggiorna la tensione
  now = millis();
  if (schermataComando) {
    if (now - inizioSchermata >= DURATA_SCHERMATA) {
      if (inGara) schermataGara();
      else schermataStandby();
    }
  } else if (!inGara && now - ultimoAggBatt >= INTERVALLO_BATT) {
    ultimoAggBatt = now;
    aggiornaVoltaggio(v);
  }
}

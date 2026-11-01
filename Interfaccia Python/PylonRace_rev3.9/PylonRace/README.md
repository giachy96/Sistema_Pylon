# Pylon Race — Centrale PC (rev3.9)

Sostituisce `Sitema_Interfaccia_PC_Centrale.xlsm`: stessa gestione gara, ma programma autonomo con
interfaccia grafica e collegamento seriale diretto alla centrale (non serve più Data Streamer).

## Avvio
- Sviluppo: `pip install -r requirements.txt` poi `python run.py [gara.json]`
- Eseguibile Windows: doppio clic su `build_exe.bat` → `dist\PylonRace_rev3.9.exe` (va creato su Windows).
- Test: `python -m pytest tests -q`

## Primo utilizzo
1. **File → Importa dal vecchio file Excel** per portare piloti, ordine di volo e risultati.
2. **Impostazioni**: token e chat id Telegram, voce, cartella dei file generati.
3. Scegli la porta (il convertitore CH340 viene proposto da solo) e premi **Connetti**.
   Senza centrale usa "Centrale simulata": ha pulsanti per provare un'intera gara.
4. Scheda **Gara**: AVANTI/INDIETRO (o i pulsanti della centrale), SHOW, START, STOP.
   I risultati arrivano dalla seriale e diventano punteggi; le celle sono modificabili a mano.

La gara si salva da sola (JSON, con copia `.bak`).

**Vista**: Ctrl++ / Ctrl+- cambiano la dimensione del testo (tutto si ridisegna di conseguenza), Ctrl+0 la riporta automatica, F11 schermo intero.

## Differenze rispetto a Excel
- Niente polling: le righe della centrale sono lette appena arrivano.
- Lo scarto si applica come in Excel: 1/2/3 peggiori manche dopo 4/8/12 manche complete.
- Sorteggio: "controllo pilota-meccanico", "casuale senza controllo" o "mantieni ordine elenco".
- Classifica con separazione FAI / non FAI opzionale, export PDF ed Excel.
- Pausa e attesa dopo reset configurabili; nomi senza accenti (il display LCD non li mostra).

## Protocollo seriale (9600 8N1, righe terminate da LF)
- PC → centrale: `codice,gara,manche,round,nome1,cognome1,nome2,cognome2,nome3,cognome3`
  (1400 reset, 850 nomi avanti, 750 indietro, 2000 show, 3000 start, 6000 stop). Righe ≤ 98 caratteri
  (il buffer della centrale è di 100 byte senza controlli).
- Centrale → PC: `800`/`700` pulsanti; risultati `5514/5524/5534` regolari, `6514/…` timeout,
  `4015/…` doppio taglio: `codice,t1..t10,totale`; colore = cifra centrale − 1 (rosso, verde, blu).
- Punteggio: 0 tagli = totale; 1 taglio = totale × 1,1; 2+ tagli, timeout, doppio taglio = 200.

## Telegram: sicurezza
Il vecchio VBA conteneva token e chat id in chiaro. **Rigenera il token con @BotFather** (il vecchio
va considerato compromesso). Nel programma il token resta solo in `config.json` del PC, mai nel file gara.

## Windows
Se la centrale non risponde, verifica l'opzione DTR in Impostazioni (Data Streamer la attivava).
La voce usa SAPI5 tramite pyttsx3.

## Messaggi Telegram personalizzabili
Impostazioni → Telegram → **Configura i messaggi…**: puoi cambiare i testi (partenza, risultati, classifica).
Segnaposto tra graffe: `{gara} {manche} {round} {rosso} {verde} {blu} {rifornimento}` (partenza),
`{emoji} {nome} {giri} {totale} {tagli} {punteggio} {lap1}…{lap10}` (risultati), `{link}` (classifica).
I testi sono in `config.json` del PC; quelli non modificati restano i predefiniti.

## Modifiche manuali all'ordine di volo (scheda Gara)
- **Sposta pilota**: premi il pulsante, clicca il pilota da spostare e poi quello con cui scambiarlo
  (stessa categoria e manche; round e colori possono essere diversi). Conferma nella finestra di riepilogo.
- **Sposta round**: clicca una riga del round da spostare, poi una riga del round di riferimento e scegli "Prima" o "Dopo"
  (anche tra categorie diverse). I numeri di round non cambiano.
- **Annulla modifica**: annulla l'ultima modifica (si può ripetere); non è possibile se nel frattempo sono arrivati risultati.
- Non si può spostare ciò che ha già risultati, né il round in corso, né inserire un round prima del round in corso.

### Reflight
- **Nuovo reflight**: scegli i piloti da rivolare (anche di manche diverse) e la posizione. Per ogni categoria
  si creano uno o più round (3 piloti l'uno, slot liberi con i fantasma). Alla centrale va il nome gara di sempre
  (es. `Q500`) con manche **100** (poi 101, 102…) e il round progressivo della categoria.
- Quando il reflight è volato si apre la conferma: vedi i valori nuovi accanto a quelli da sostituire e puoi cambiare
  il volo da sostituire. Se scegli "Più tardi", usa **Conferma reflight**. **Annulla modifica** ripristina il vecchio valore.
- La classifica usa solo i valori sostituiti, non rilegge i round di reflight.
- Esc annulla la selezione in corso. Se uno scambio riguarda il round in volo, i nomi vengono rinviati alla centrale.

### Classifica su Telegram
- Con Telegram attivo, la classifica (a mano o automatica) viene inviata come file PDF direttamente nella chat; il testo del messaggio "classifica" è la didascalia. Per tornare al solo link, spegni la casella "PDF su Telegram" nelle Impostazioni (gruppo Gara, accanto al link classifica).
- **Aggiornamento automatico** (scheda Classifica): a mano, a fine di ogni manche, a fine di ogni categoria della manche, oppure ogni N minuti. Dopo la conferma di un reflight la classifica si aggiorna da sola (modalità manche/categoria).

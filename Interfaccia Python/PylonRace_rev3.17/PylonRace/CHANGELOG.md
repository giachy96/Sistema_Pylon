# Versioni
Numerazione: 3.x per gli aggiornamenti normali, 4 per una versione major.

## rev3.17
- Impostazioni su due colonne affiancate (sinistra: Gara e Centrale; destra: Telegram e Voce): tutto visibile senza
  scorrere, anche su un portatile 1366×768. Testo del flag PDF accorciato ("Invia il PDF della classifica su Telegram"),
  la spiegazione è nel suggerimento al passaggio del mouse.

## rev3.16
- Secondo check, su scenari "da campo":
  - File gara danneggiato (es. PC spento durante il salvataggio): prima il programma ripartiva vuoto e al primo salvataggio
    automatico cancellava anche la copia di sicurezza buona. Ora riapre l'ultima copia di sicurezza (.bak), conserva il file
    rovinato come "gara.danneggiato-<data>.json" e avvisa. La copia di sicurezza si aggiorna solo partendo da un file sano.
  - config.json (token, chat, porta...) salvato in modo sicuro con copia di riserva: se si rovina si usa la copia.
  - Risultati che arrivano su un round che li aveva già: avviso nel registro e i valori precedenti si recuperano con Annulla.
  - Valori a mano non validi (0, negativi, "nan", "inf") nel totale o nel punteggio vengono rifiutati invece di falsare la
    classifica.
  - Righe sporche o incomplete dalla seriale (3000 righe casuali): nessun errore.

## rev3.15
- Check approfondito. Corretto: dopo aver rigenerato il sorteggio, "Annulla" poteva ancora ripristinare un vecchio valore
  (reflight confermato, scambio o spostamento) dentro il nuovo ordine di volo. Ora la cronologia degli annulla viene azzerata
  alla rigenerazione. Pulizia di import inutilizzati.

## rev3.14
- Sorteggio: l'ordine delle categorie si sceglie con 4 menu a tendina in fila (1ª, 2ª, 3ª, 4ª) al posto delle 4 combinazioni
  fisse. Una categoria scelta sparisce dagli altri menu; l'ultima posizione si riempie da sola. Si può svuotare una posizione
  per cambiare idea; per generare servono tutte e 4 le posizioni.

## rev3.13
- Piloti: un colore per ogni coppia pilota/meccanico. Lo stesso colore compare su Nome e Cognome del pilota che fa anche
  da meccanico e su Nome mecc. / Cognome mecc. di chi lo ha come meccanico (suggerimento sulla cella). I colori ripartono
  da capo per ogni gara (12 colori disponibili). Sostituisce il solo giallo della 3.12.

## rev3.12
- Piloti: tolta l'evidenziazione "iscritto due volte"; resta solo il giallo pilota-meccanico.

## rev3.11
- Piloti e Classifica: le colonne si regolano a mano trascinando le intestazioni; le larghezze si ricordano
  (anche dopo la chiusura del programma, per PC). Le colonne M1, M2… della classifica hanno la stessa larghezza.
- Piloti: Nome e Cognome in **giallo** se il pilota è anche meccanico di un altro pilota. Il suggerimento sulla cella
  dice di chi; legenda in fondo alla scheda. Il confronto ignora spazi e maiuscole.

## rev3.10
- Schermi grandi: la dimensione automatica del testo ora cresce con l'altezza dello schermo (Full HD 11,5 pt,
  1440p 13 pt, 4K 15 pt). Il menu Vista continua a permettere di cambiarla.
- Scheda Gara: la colonna dei comandi non taglia più i pulsanti (Reset, simulatore in una colonna, "Avanzamento" su riga propria);
  la tabella si adatta alla larghezza disponibile: Tagli e Punti restano visibili, si stringono prima i giri poi i nomi.

## rev3.9
- Impostazioni: la casella "Invia il PDF della classifica direttamente su Telegram" è ora nel gruppo "Gara",
  insieme al link classifica (non più nel gruppo Telegram). Corretto un errore: all'avvio le Impostazioni
  potevano spegnerla da sole.

## rev3.8
- Scheda Classifica: il pulsante **Telegram** ora genera la classifica aggiornata e invia il PDF
  (prima mandava solo il testo con il link). Impostazioni: la casella "Invia il PDF…" si salva subito.

## rev3.7
- **Classifica automatica a fine manche o fine categoria**: scheda Classifica → "Aggiornamento automatico":
  *Solo a mano* · *A fine di ogni manche* (tutte le categorie hanno volato) · *A fine di ogni categoria della manche*
  (tutti i round di una categoria in quella manche) · *Ogni N minuti* (come prima). Genera il PDF e, con Telegram attivo,
  lo invia nella chat. Dopo la conferma di un reflight la classifica si aggiorna nelle prime due modalità.
  I file vecchi con l'aggiornamento a tempo restano a tempo.

## rev3.6
- **Classifica su Telegram come PDF**: il file `Classifica.pdf` viene inviato direttamente nella chat (sendDocument),
  senza bisogno di caricarlo su Google. Il testo del messaggio "classifica" diventa la didascalia del PDF.
  Impostazioni → Telegram: casella "Invia il PDF della classifica direttamente su Telegram"; se spenta, si invia
  solo il testo con il link come prima.

## rev3.5
- **Reflight**: pulsanti *Nuovo reflight* e *Conferma reflight* nella scheda Gara.
  - Scegli uno o più piloti già volati (anche di manche diverse) e la posizione nell'ordine di volo
    (mai prima del round in corso o già volato). I reflight sono per categoria; oltre 3 piloti → più round,
    gli slot liberi si riempiono con i piloti fantasma.
  - Il nome della gara non cambia. Il reflight è identificato dal numero di manche: 100 per il primo, 101 per il secondo…
    (round da 1 per ogni categoria). Nel programma compare come "RF1", "RF2"… e nel messaggio Telegram come "(Reflight 1)", "(Reflight 2)"…
  - Colore di partenza: resta l'originale se non c'è conflitto, altrimenti R, V, B nell'ordine di scelta.
  - Finito il volo si apre la conferma: valori del reflight (tempi, totale, tagli, punti) accanto a quelli da sostituire,
    con il nome del pilota; si può cambiare il volo da sostituire. Il valore sostituito è conservato e **Annulla modifica**
    lo ripristina.
  - La classifica ignora i round di reflight e legge solo i valori sostituiti negli slot originali.
  - Il contatore "n / N round" comprende anche i reflight. Telegram: nuovo segnaposto {reflight}.

## rev3.4
- Scheda Gara: **Sposta pilota** (scambio di due piloti della stessa categoria e manche, anche in round diversi) e
  **Sposta round** (un'intera tripletta prima/dopo un altro round, anche di un'altra categoria), con **Annulla modifica**.
  I numeri di round restano invariati. Bloccato tutto ciò che ha già risultati; non si inserisce prima del round in corso.
  Avviso se lo scambio crea un conflitto pilota-meccanico; se tocca il round in volo, i nomi vengono rinviati alla centrale.

## rev3.3
- Scheda Gara: il contatore dei round ("Avanzamento: 7 / 114 round") ora è visibile e chiaramente etichettato.

## rev3.2
- Impostazioni → Telegram → "Configura i messaggi…": testi di partenza, risultati e classifica modificabili,
  con segnaposto ({gara}, {manche}, {rosso}…), anteprima e ripristino dei predefiniti. Salvati per PC.

## rev3.1
- Scheda Gara: comandi INDIETRO / AVANTI / SHOW / START / STOP sempre visibili in alto, anche su schermi piccoli.
- Caselle dei piloti in volo più compatte (2 righe); pulsanti "Parti da qui" e "Reset" accorciati.

## rev3
- Sorteggio: tre modalità — controllo pilota-meccanico, casuale senza controllo, mantieni ordine elenco.
- Logo FT Models, tema scuro, dimensioni che seguono il testo (menu Vista), esportazione griglia in Excel,
  tripletta in volo rosso/verde/blu, modello di importazione piloti.

## rev1
- Prima versione: porting completo del sistema Excel/VBA.

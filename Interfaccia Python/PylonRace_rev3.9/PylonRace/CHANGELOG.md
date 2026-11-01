# Versioni
Numerazione: 3.x per gli aggiornamenti normali, 4 per una versione major.

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

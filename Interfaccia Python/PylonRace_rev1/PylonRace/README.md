# Pylon Race — Centrale PC (rev1)

Sostituisce `Sitema_Interfaccia_PC_Centrale.xlsm`: stessa gestione gara, ma programma autonomo con
interfaccia grafica e collegamento seriale diretto alla centrale (non serve più Data Streamer).

## Avvio
- Sviluppo: `pip install -r requirements.txt` poi `python run.py [gara.json]`
- Eseguibile Windows: doppio clic su `build_exe.bat` → `dist\PylonRace_rev1.exe` (va creato su Windows).
- Test: `python -m pytest tests -q`

## Primo utilizzo
1. **File → Importa dal vecchio file Excel** per portare piloti, ordine di volo e risultati.
2. **Impostazioni**: token e chat id Telegram, voce, cartella dei file generati.
3. Scegli la porta (il convertitore CH340 viene proposto da solo) e premi **Connetti**.
   Senza centrale usa "Centrale simulata": ha pulsanti per provare un'intera gara.
4. Scheda **Gara**: AVANTI/INDIETRO (o i pulsanti della centrale), SHOW, START, STOP.
   I risultati arrivano dalla seriale e diventano punteggi; le celle sono modificabili a mano.

La gara si salva da sola (JSON, con copia `.bak`).

## Differenze rispetto a Excel
- Niente polling: le righe della centrale sono lette appena arrivano.
- Lo scarto si applica come in Excel: 1/2/3 peggiori manche dopo 4/8/12 manche complete.
- Sorteggio: modalità "come Excel", "senza conflitti pilota/meccanico" o casuale.
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

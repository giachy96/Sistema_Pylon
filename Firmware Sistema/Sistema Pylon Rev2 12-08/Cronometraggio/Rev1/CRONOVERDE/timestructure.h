float lontoflt ( unsigned long loong ) {
  float flt;
  loong = loong;
  flt = (float)loong;
  flt = flt / 100;
  return flt;
}

// sscanf Read formatted data from string (function ) ci potrebbe servire per leggere una riga formattata

void Catturatempo ( float* tempo_flt, long* tempo_parziale, int indice , unsigned long tempobase) {
  unsigned long tempo_appoggio = 0;
  unsigned long tempo_cronometrato;
 //Serial.println("sono dentro il codice cattura");
  if (indice < 0) {
    memset(*(tempo_parziale), 0, sizeof(*(tempo_parziale)));
    tempo_appoggio = 0;
    Serial.println("minore"); // lo devo lasciare perchè senno mi salta lo 0 non capisco come mai 
  }
  else if (indice == 0) {
    tempo_cronometrato = millis() - tempobase;
     tempo_cronometrato = tempo_cronometrato/10; // per ridurre al centesimo
    *(tempo_parziale ) =  tempo_cronometrato;
    *(tempo_flt ) = lontoflt(tempo_parziale[0] );
   // Serial.println("uguale");

  }
  else if ( indice > 0) {
    tempo_cronometrato = millis() - tempobase;
    tempo_cronometrato = tempo_cronometrato/10; // per ridurre al centesimo
    *(tempo_parziale + indice) =  tempo_cronometrato;
    for (int i = 1; i <= indice  ; i++) {

      tempo_appoggio = tempo_appoggio + *(tempo_parziale + i - 1); 
    }
    *(tempo_parziale + indice) = *(tempo_parziale + indice) - tempo_appoggio;
    *(tempo_flt + indice) = lontoflt( tempo_parziale[indice]);
    //Serial.println("magg");
  }
 

}

String conversionecommastr ( String colore , long* tempo_parziale) {
  String result = colore;
  for (int j =0 ; j <= 9; j++) {  
      result.concat(",");
      result.concat(String(*(tempo_parziale +j)));
  }

return result;

}
float  CalcoloTempo (float poitemp[]) {
  float tempotot = 0;
  for (int k = 0; k <= 10; k ++) {
    tempotot = tempotot + poitemp[k];
  }
  // Serial.println(tempotot);
  return tempotot;
}

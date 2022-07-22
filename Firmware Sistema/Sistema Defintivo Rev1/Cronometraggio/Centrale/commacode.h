
char* arr[10];


String conversionecommastr (String colore , long tempo_parziale[]) {
  String result = colore;
  for (int j=0 ; j <= 5; j++) {
      result.concat(",");
      result.concat(String(tempo_parziale[j]));
  }
  return result;
}

int decodecomma (String str , int tempi[]){
   int lungh_str = str.length();
   char buff[lungh_str+1];
    str.toCharArray(buff, lungh_str+1);
   //Serial.println(parseData(buff));
      int i =0;
      char* p;
  //Serial.println("Example of splitting a string into tokens: ");
 // Serial.print("The input string is: '");
 // Serial.print(buff);
 // Serial.println("'");

  p = strtok(buff, "{,}"); //2nd argument is a char[] of delimiters
  while (p != '\0') { //not equal to NULL
    //Serial.println(p);
     tempi[i] = atoi(p);
     
    p = strtok('\0', "{,}");  //expects NULL for string on subsequent calls
    i++;
  }

  return tempi;
  }

/*void setup() {
  Serial.begin(9600);
  delay(500);
  Serial.println("ciao");
  long ray[] = {22222, 3333, 555, 66, 99, 6688};
  String col = "77";
  Serial.println(String(ray[1]));
  String str = conversionecommastr(col , ray);
  Serial.println(str);
  str.concat(",");
   Serial.println(str);
   String tmp = String(ray[2]);
   str.concat(tmp);
   Serial.println(str);


// DA STRING A INTERI
   
   char buff[50];
    str.toCharArray(buff, 50);
   //Serial.println(parseData(buff));

      int tempi_rx[12] ;
      int i =0;
      char* p;
  Serial.println("Example of splitting a string into tokens: ");
  Serial.print("The input string is: '");
  Serial.print(buff);
  Serial.println("'");

  p = strtok(buff, "{,}"); //2nd argument is a char[] of delimiters
  while (p != '\0') { //not equal to NULL
    Serial.println(p);
     tempi_rx[i] = atoi(p);
     
    p = strtok('\0', "{,}");  //expects NULL for string on subsequent calls
    i++;
  }

  Serial.println("Stampa p");
  for(int k=0; k<= 7;k++){
     Serial.println(tempi_rx[k] );
    }
 
}
*/

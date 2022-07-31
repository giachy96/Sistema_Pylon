#include <Wire.h>
# define I2C_SLAVE_ADDRESS 11 // 12 pour l'esclave 2 et ainsi de suite
#define PAYLOAD_SIZE 2


#include <SPI.h>
#include <DMD2.h>
#include <fonts/SystemFont5x7.h>


SoftDMD dmd(1,1); 


void setup()
{  dmd.setBrightness(255);
  dmd.selectFont(SystemFont5x7);
  dmd.begin();
  Wire.begin(I2C_SLAVE_ADDRESS);
  Serial.begin(9600);
  Serial.println("-------------------------------------I am Slave1");
  delay(1000);
  Wire.onRequest(requestEvents);
  Wire.onReceive(receiveEvents);
}
void loop() {}
int n = 0;
void requestEvents()
{
  Serial.println(F("---> recieved request"));
  Serial.print(F("sending value : "));
  Serial.println(n);
  Wire.write(n);
}
void receiveEvents(int numBytes)
{
  Serial.println(F("---> recieved events"));
  n = Wire.read();
  Serial.print(numBytes);
  Serial.println(F("bytes recieved"));
  Serial.print(F("recieved value : "));
  Serial.println(n);
}

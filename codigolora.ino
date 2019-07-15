#include <SPI.h> 
#include <LoRa.h> 
#include <Wire.h>  
#include "SSD1306.h" 
#include <SimpleDHT.h>
 

 
#define DHTTYPE DHT11   
 

#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     14   // GPIO14 -- SX127x's RESET
#define DI00    26   // GPIO26 -- SX127x's IRQ(Interrupt Request)
 
#define humedad 12
#define humedaddig 13

#define BAND    433E6  
const int DHTPin = 23;
int analog_value;
int dig_value;
int hum_limit = 0;
float h, c;
String packSize;
String packet = "OK";
String values = "|-|-";
SSD1306 display(0x3c, 4, 15);
void setup()
{
  h = c = 0;
  Serial.begin(9600);
  pinMode(humedad, INPUT);
  pinMode(humedaddig,INPUT);

  pinMode(16,OUTPUT); 
 
  digitalWrite(16, LOW);    
  
  delay(50); 
  
  digitalWrite(16, HIGH); 
 
  display.init(); 

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10); 
  
  delay(1500);
  
  display.clear();
   
  SPI.begin(SCK,MISO,MOSI,SS); 

  LoRa.setPins(SS,RST,DI00); 
   
  
  if (!LoRa.begin(BAND))
  {
    
    display.drawString(0, 0, "Starting LoRa failed!");
 
    display.display(); 
    
    while (true);
  }      

}

void loop()
{
  
  display.clear();

  display.setTextAlignment(TEXT_ALIGN_LEFT);
  
  display.setFont(ArialMT_Plain_16);
 
  display.drawString(0, 0, "Running...");
  
  readDhtSensor();  

  values="|"+String(h)+"|"+String(c);  

  if(gasDetected())
  {
  
    packet = "ALARM";   
  
    packet+=values;
  
    sendPacket();

    showDisplay(true);
  }
  else
  { 
    
    packet = "OK"; 
  
    packet+=values;
   
    sendPacket();

    showDisplay(false);
  }
 
  delay(250);
}
void showDisplay(bool gasDetected)
{
  
  display.clear();
 
  display.setTextAlignment(TEXT_ALIGN_LEFT);

  display.setFont(ArialMT_Plain_16);

  display.drawString(0, 0, "Running...");
 
 
  if(gasDetected)
  {
    
    display.drawString(0, 20, "Status: ALARM");
 
    display.drawString(0, 40, "Gas detected!");
   
    display.display(); 
  }  
  else
  {
    //escreve na pos 0,20 a mensagem
    display.drawString(0, 20, "Status: OK");    
    //escreve na pos 0,40 a mensagem  
    display.drawString(0, 40, "H: "+String(h)+" T: "+String(c)+"°");   
    //exibe no display
    display.display(); 
  }
}

bool gasDetected()
{  
 
  analog_value = analogRead(humedad);
  dig_value = digitalRead(humedaddig); 
  Serial.print(analog_value);
  
  Serial.print(" || ");
 if(dig_value == 0)
  {
    if(hum_limit == 0 || hum_limit > analog_value)
      hum_limit = analog_value;
 
  
    Serial.println("GAS DETECTED !!!");
   
    Serial.println("Gas limit: "+String(hum_limit));
 
  
    return true;
  }
  else
  {
    //exibe na serial "Gas ausente" 
    Serial.println("No gas detected...");
 
    //exibe o limite mínimo default "X" caso não foi acionado pela primeira vez
    if(hum_limit == 0)
      Serial.println("Hum limit: X");
    else //exibe o limite mínimo de gás captado para o evento ocorrer
      Serial.println("Gas limit: "+String(hum_limit));
 
    //retorna falso
    return false;
  }
}
void readDhtSensor()
{
 
  float novoC, novoH;
 
  delay(250);

  int err = DHT22.read2(DHTPin, &novoC, &novoH, NULL);

  if (err != SimpleDHTErrSuccess) 
  {
    
    Serial.print("Read DHT22 failed, err="); 
    Serial.println(err);
    return;
  }

  c = novoC;
  h = novoH;
 
  Serial.print((float)c); Serial.println(" *C ");
  Serial.print((float)h); Serial.println(" H");

  delay(250);
}
void sendPacket()
{
  
  LoRa.beginPacket();

  LoRa.print(packet);

  LoRa.endPacket(); 
}

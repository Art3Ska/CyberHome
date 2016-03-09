#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include <DHT.h>

#define DHTPIN 2
#define DHTTYPE DHT22   // DHT 22  (AM2302)

DHT dht(DHTPIN, DHTTYPE);

RF24 radio(7, 8);

const byte rxAddr[6] = "00001";

void setup()
{
  while (!Serial);
  Serial.begin(9600);
  radio.begin();
  radio.setRetries(15, 15);
  radio.openWritingPipe(rxAddr);

  radio.stopListening();
  dht.begin();
}

void loop()
{
  //
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  char text[32] = "";
  char charVal[10];               //temporarily holds data from vals
  String s = "";
  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t) || isnan(h)) {
    strcpy(text, "BLAD odczytu");
  } else {
    s = "H: " + String(h) + " T: " + String(t);
    //  dtostrf(h, 4, 4, charVal);
    //  strcpy(text, "HUM: ");
    //   strcpy(text, charVal);
    //  dtostrf(t, 4, 4, charVal);
    //   strcpy(text, " TMP: ");
    //strcpy(text, charVal);
    strcpy(text, s.c_str());
  }
  //

  radio.write(&text, sizeof(text));
  Serial.println(text);
  delay(3000);
}


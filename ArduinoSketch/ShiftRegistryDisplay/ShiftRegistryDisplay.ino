/*Rejestr przesuwny
  #--Rejestr1 1A 2B 3C 4D 5E 6F 7G 8D1 --Rejestr2 9(1)BUZER 10(2)SEGMENT1 11(3)SEGMENT2 12(4)SEGMENT3 13(5)SEGMENT4 14(6)DIODA1 15(7)DIODA2 16(8)DIODA3
  #PINY*/
int DATA_P = 7;
int RCLK_P = 6;
int SRCLK_P = 2;
int i = 0;
byte __SEGMENTS[5];
byte __DIGITS[11];
byte __DIODES[4];

void setup() {
  Serial.begin(9600);

  __SEGMENTS[0] = 0b00000000;
  __SEGMENTS[1] = 0b00000010;
  __SEGMENTS[2] = 0b00000100;
  __SEGMENTS[3] = 0b00001000;
  __SEGMENTS[4] = 0b00010000;

  __DIGITS[0] = 0b11111101;
  __DIGITS[1] = 0b01100001;
  __DIGITS[2] = 0b11011011;
  __DIGITS[3] = 0b11110011;
  __DIGITS[4] = 0b01100111;
  __DIGITS[5] = 0b10110111;
  __DIGITS[6] = 0b10111111;
  __DIGITS[7] = 0b11100001;
  __DIGITS[8] = 0b11111111;
  __DIGITS[9] = 0b11110111;
  __DIGITS[10] = 0b00000010; /*-*/
  __DIGITS[11] = 0b00000000; /*space*/

  __DIODES[0] = 0b10000000; /*czerwona*/
  __DIODES[1] = 0b00100000; /*zielona*/
  __DIODES[2] = 0b01000000; /*niebieska*/
  __DIODES[3] = 0b00000001; //DZWONEK

  pinMode (DATA_P, OUTPUT);
  pinMode (RCLK_P, OUTPUT);
  pinMode (SRCLK_P, OUTPUT);
}

void loop() {
  EmptyScreen();
  if (i == 9999) i = 0;
  Write(String(i), 1, 0, 0, 0);
  if (i % 10 == 0)
    Write(String(i), 0, 1, 1, 1);
  i++;
}

byte LedLight(byte data, bool red, bool green, bool blue, bool ring) {
  byte d = 0b00000000;
  if (red)
    d = __DIODES[0];
  if (green)
    d = d | __DIODES[1];
  if (blue)
    d = d | __DIODES[2];
  if (ring)
    d = d | __DIODES[3];
  return data | d;
}

void Write(String txt, bool redDiode, bool greenDiode, bool blueDiode, bool ring)
{
  byte d;
  for (int i = 0; i < txt.length(); i++)
  {
    int code = String(txt[i]).toInt();
    //Serial.println(txt.length());
    d = __SEGMENTS[i + 1];
    d = LedLight(d, redDiode, greenDiode, blueDiode, ring) ;
    digitalWrite(RCLK_P, LOW);
    shiftOut (DATA_P, SRCLK_P, MSBFIRST , d);
    shiftOut (DATA_P, SRCLK_P, LSBFIRST , __DIGITS[code]);
    digitalWrite(RCLK_P, HIGH);
    delay(10);
  }
}

void Write2()
{
  digitalWrite(RCLK_P, LOW);
  shiftOut (DATA_P, SRCLK_P, MSBFIRST , __SEGMENTS[1] | __DIODES[2]);
  shiftOut (DATA_P, SRCLK_P, LSBFIRST , __DIGITS[5]);
  digitalWrite(RCLK_P, HIGH);
  delay(10);
}


void EmptyScreen()
{
  digitalWrite(RCLK_P, LOW);
  shiftOut (DATA_P, SRCLK_P, MSBFIRST, 0 << 8);
  shiftOut (DATA_P, SRCLK_P, MSBFIRST, 0 << 8);
  digitalWrite(RCLK_P, HIGH);
}


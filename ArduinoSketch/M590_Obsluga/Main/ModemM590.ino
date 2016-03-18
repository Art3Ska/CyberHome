#define DEBUG_ON
int PROBE_INTERWAL_SEC = 1;

#define pinRX  6
#define pinTX  7
#define restartModemPin 5

#define ModemSerialSpeed 4800 //4800

SoftwareSerial gsm(pinRX, pinTX);
int probeCount = 0;

void DelayAdv(long sec)
{
  unsigned long previousMillis = millis();
  while (true)
  {
    //Serial.println(millis());
    //Serial.println(previousMillis);
    if ((unsigned long)(millis() - previousMillis) > sec)
      break;
  } //end while
  //  delay(sec);
}

void ModemInit() {
#ifdef DEBUG_ON
  Serial.begin(115200); //komp
  while (!Serial);
#endif

  pinMode(restartModemPin, OUTPUT);
  digitalWrite(restartModemPin, LOW);

  gsm.begin(ModemSerialSpeed); //modem
  while (gsm.available()); //wsztrzymaj dopoki serial nie bedzie gotowy
}

void ModemSend(int SensorId, String Message) {
#ifdef DEBUG_ON
  Serial.println("______________START______________");
#endif
  bool ok = false;
  WriteAdv(gsm, 1, "AT+XIIC?\r", "10.", ok, 0);
#ifdef DEBUG_ON
  Serial.println("-------------- Polaczenie z APN: " + String(ok));
#endif
  if (!ok)
    PrepareGprs(gsm);

  probeCount = 0;
  SendGprsData(gsm, SensorId, Message);

  //  gsm.write("\r\n\r\n");
  //  Serial.println("OUTres: " +  gsm.readString());
  //  ReadSimple(gsm, 3);
}

bool PrepareGprs(SoftwareSerial& ser)
{
  probeCount++;
  bool ok;
  WriteAdv(gsm, 2, "ATE0\r", "OK", ok, 0);
  WriteAdv(gsm, 5, "AT+CREG?\r", "0,1", ok, 5);

  if (!ok)
  {
    if (probeCount >= 10)
    {
      ModemSleep();
      probeCount = 0;
    }
    ModemRestart();
    PrepareGprs(ser);
    return 0;
  }

  WriteAdv(gsm, 2, "AT+XISP=0\r", "OK", ok, 0);
  WriteAdv(gsm, 2, "AT+CGDCONT=1,\"IP\",\"internet\"\r", "OK", ok, 0);
  WriteAdv(gsm, 2, "AT+XIIC=1\r", "OK", ok, 10);
  WriteAdv(gsm, 5, "AT+XIIC?\r", "10.", ok, 8);

  if (!ok)
  {
    if (probeCount >= 10)
    {
      ModemSleep();
      probeCount = 0;
    }
    ModemRestart();
    PrepareGprs(ser);
    return 0;
  }
  probeCount = 0;
  return ok;
}

bool SendGprsData(SoftwareSerial& ser, int SensorId, String Message)
{
  bool ok;
  probeCount++;
  WriteAdv(gsm, 1, "AT+IPSTATUS=0\r", "TCP", ok, 0);
#ifdef DEBUG_ON
  Serial.println("___Polaczenie z moim IP: " + String(ok));
#endif
  if (!ok) // jesli nie polaczony z moim IP
  {
    WriteAdv(gsm, 2, "AT+TCPSETUP=0,104.40.147.___,80\r", "OK", ok, 0);
    WriteAdv(gsm, 5, "AT+IPSTATUS=0\r", "TCP", ok, 0);
  }

  if (!ok)
  {
    ModemRestart();
    PrepareGprs(ser);
    SendGprsData(ser, SensorId, Message);
    return 0;
  }
  String strC0 = "GET /api/BusApi/Save" + PrepareMessage(SensorId, Message) + " HTTP/1.1\r\n";
  int lenC0 = strC0.length();

  String strC1 = "AT+TCPSEND=0," +  String(35 + lenC0) + "\r";

#ifdef DEBUG_ON
  Serial.println("strC1: " + String(strC1));
#endif

  WriteAdv(gsm, 3, strC1, ">", ok, 1);
  if (!ok)
  {
    ModemRestart();
    PrepareGprs(ser);
    SendGprsData(ser, SensorId, Message);
    return 0;
  }

#ifdef DEBUG_ON
  Serial.println("strCO: " + strC0);
#endif

  WriteAdv(gsm, 1, String(strC0), "", ok, 1);
  WriteAdv(gsm, 1, "HOST:___.azurewebsites.net\r\n", "", ok, 1);
  // gsm.write("\r\n\r\n");
  // ReadSimple(gsm, 3);
  WriteAdv(gsm, 5, "\r\n\r\n", "REC", ok, 5);

  if (!ok && probeCount < 2)
  {
    WriteAdv(gsm, 1, "AT+TCPCLOSE=0", "", ok, 5);
    SendGprsData(ser, SensorId, Message);
    return 0;
  }

  if (!ok) //TODO:
  {
    probeCount = 0;
    ModemRestart();
    PrepareGprs(ser);
    SendGprsData(ser, SensorId, Message);
    return 0;
  }
}

String WriteAdv(SoftwareSerial& ser, int probeCount, String command, String recResp, bool& res, int ProbeInterwalSec)
{
  if (ProbeInterwalSec == 0)
    ProbeInterwalSec = PROBE_INTERWAL_SEC;
  res = true;
  String resp = "NN";
  for (int i = 0; i < probeCount; i++)
  {
    DelayAdv(1000 * PROBE_INTERWAL_SEC);
#ifdef DEBUG_ON
    Serial.println("WRITE command: " + command);
#endif
    char buf[500] = "";
    command.toCharArray(buf, 500);

    ser.write(buf);
    resp = ReadSimple(ser, 2); //TODO: czytaj dopoki modem nadaje?? Produkcyjnie wylaczyc echo modemu (dlatego dwa, raz echo, drugi raz odp.)
    if (resp.indexOf(recResp) > 0 || recResp == "")
    {
      return resp;
    }
    DelayAdv(1000 * ProbeInterwalSec);
  }//end for
#ifdef DEBUG_ON
  Serial.println("RESP NOT OK: " + String(command) + " resp:" + resp);
#endif
  res = false;
  return resp;
}

String ReadSimple(SoftwareSerial& ser, int probeCount)
{
  String out;
  String resp;

  for (int i = 0; i < probeCount; i++)
  {
    DelayAdv(1000 * PROBE_INTERWAL_SEC);
    resp = ser.readString();
#ifdef DEBUG_ON
    Serial.println("  **resp part: " + resp);
#endif
    if (resp != "")
      if (probeCount > 1)
        out += resp + "\r\n";
      else
        out = resp;
  }
#ifdef DEBUG_ON
  Serial.println("RES: " + out);
#endif
  return out;
}

String GetJsonResponse(SoftwareSerial& ser, int probeCount)
{
  //TODO
}

void ModemRestart()
{
  Serial.println("___Procedura ponawiania___");
  digitalWrite(restartModemPin, LOW);
  digitalWrite(restartModemPin, HIGH);
  DelayAdv(1000 * 2);
  digitalWrite(restartModemPin, LOW);
  DelayAdv(1000 * 10);
}

void ModemSleep()
{
  Serial.println("___Uspienie modemu na 1h___");
  DelayAdv(1000 * PROBE_INTERWAL_SEC * 5);
  probeCount = 0;
  //TODO: SLEEP MODEM
}

String PrepareMessage(int SensorId, String Message)
{
  String msg = "";
  msg += "?sid=" + String(SensorId);
  msg += "&msg=" + Message;
  msg += "&tkn=___";
  return msg;
}



#include <WiFiClient.h>
#include "ESP32_FTPClient.h"

ESP32_FTPClient::ESP32_FTPClient(char *_serverAdress, uint16_t _port, char *_userName, char *_passWord, uint16_t _timeout, uint8_t _verbose)
{
  userName = _userName;
  passWord = _passWord;
  serverAdress = _serverAdress;
  port = _port;
  timeout = _timeout;
  verbose = _verbose;
}

ESP32_FTPClient::ESP32_FTPClient(char *_serverAdress, char *_userName, char *_passWord, uint16_t _timeout, uint8_t _verbose)
{
  userName = _userName;
  passWord = _passWord;
  serverAdress = _serverAdress;
  port = 21;
  timeout = _timeout;
  verbose = _verbose;
}

WiFiClient *ESP32_FTPClient::GetDataClient()
{
  return &dclient;
}

bool ESP32_FTPClient::isConnected()
{
  if (!_isConnected)
  {
    FTPerr("FTP error: ");
    FTPerr(outBuf);
    FTPerr("\n");
  }

  return _isConnected;
}

void ESP32_FTPClient::getLastModifiedTime(const char *fileName, char *result)
{
  FTPdbgn("Send MDTM");
  if (!isConnected())
    return;
  client.print(F("MDTM "));
  client.println(F(fileName));
  getFTPAnswer(result, 4);
}

void ESP32_FTPClient::writeClientBuffered(WiFiClient *cli, unsigned char *data, int dataLength)
{
  if (!isConnected())
    return;

  size_t clientCount = 0;
  for (int i = 0; i < dataLength; i++)
  {
    clientBuf[clientCount] = data[i];
    //client.write(data[i])
    clientCount++;
    if (clientCount > bufferSize - 1)
    {
      cli->write(clientBuf, bufferSize);
      clientCount = 0;
    }
  }
  if (clientCount > 0)
  {
    cli->write(clientBuf, clientCount);
  }
}

void ESP32_FTPClient::getFTPAnswer(char *result, int offsetStart)
{
  char thisByte;
  outCount = 0;

  unsigned long _m = millis();
  while (!client.available() && millis() < _m + timeout)
    delay(1);

  if (!client.available())
  {
    memset(outBuf, 0, sizeof(outBuf));
    strcpy(outBuf, "Offline");

    _isConnected = false;
    isConnected();
    return;
  }

  while (client.available())
  {
    thisByte = client.read();
    if (outCount < sizeof(outBuf))
    {
      outBuf[outCount] = thisByte;
      outCount++;
      outBuf[outCount] = 0;
    }
  }

  if (outBuf[0] == '4' || outBuf[0] == '5')
  {
    _isConnected = false;
    isConnected();
    return;
  }
  else
  {
    _isConnected = true;
  }

  if (result != NULL)
  {
    FTPdbgn("Result start");
    // Deprecated
    for (int i = offsetStart; i < sizeof(outBuf); i++)
    {
      result[i] = outBuf[i - offsetStart];
    }
    FTPdbg("Result: ");
    //Serial.write(result);
    FTPdbg(outBuf);
    FTPdbgn("Result end");
  }
}

void ESP32_FTPClient::writeData(unsigned char *data, int dataLength)
{
  FTPdbgn(F("Writing"));
  if (!isConnected())
    return;
  writeClientBuffered(&dclient, &data[0], dataLength);
}

void ESP32_FTPClient::closeFile()
{
  FTPdbgn(F("Close File"));
  dclient.stop();

  if (!_isConnected)
    return;

  getFTPAnswer();
}

void ESP32_FTPClient::write(const char *str)
{
  FTPdbgn(F("Write File"));
  if (!isConnected())
    return;

  GetDataClient()->print(str);
}

void ESP32_FTPClient::closeConnection()
{
  client.println(F("QUIT"));
  client.stop();
  FTPdbgn(F("Connection closed"));
}

void ESP32_FTPClient::openConnection()
{
  FTPdbg(F("Connecting to: "));
  FTPdbgn(serverAdress);
  if (client.connect(serverAdress, port, timeout))
  {
    FTPdbgn(F("Command connected"));
  }

  getFTPAnswer();

  FTPdbgn("Send USER");
  client.print(F("USER "));
  client.println(F(userName));
  getFTPAnswer();

  FTPdbgn("Send PASSWORD");
  client.print(F("PASS "));
  client.println(F(passWord));
  getFTPAnswer();

  FTPdbgn("Send SYST");
  client.println(F("SYST"));
  getFTPAnswer();
}

void ESP32_FTPClient::renameFile(char *from, char *to)
{
  FTPdbgn("Send RNFR");
  if (!isConnected())
    return;
  client.print(F("RNFR "));
  client.println(F(from));
  getFTPAnswer();

  FTPdbgn("Send RNTO");
  client.print(F("RNTO "));
  client.println(F(to));
  getFTPAnswer();
}

void ESP32_FTPClient::newFile(const char *fileName)
{
  FTPdbgn("Send STOR");
  if (!isConnected())
    return;
  client.print(F("STOR "));
  client.println(F(fileName));
  getFTPAnswer();
}

void ESP32_FTPClient::initFile(const char *type)
{
  FTPdbgn("Send TYPE");
  if (!isConnected())
    return;
  FTPdbgn(type);
  client.println(F(type));
  getFTPAnswer();

  FTPdbgn("Send PASV");
  client.println(F("PASV"));
  getFTPAnswer();
  
  char *tStr = strtok(outBuf, "(,");
  int array_pasv[6];
  for (int i = 0; i < 6; i++)
  {
    tStr = strtok(NULL, "(,");
    if (tStr == NULL)
    {
      FTPdbgn(F("Bad PASV Answer"));
      closeConnection();
      return;
    }
    array_pasv[i] = atoi(tStr);
  }
  unsigned int hiPort, loPort;
  hiPort = array_pasv[4] << 8;
  loPort = array_pasv[5] & 255;

  IPAddress pasvServer(array_pasv[0], array_pasv[1], array_pasv[2], array_pasv[3]);

  FTPdbg(F("Data port: "));
  hiPort = hiPort | loPort;
  FTPdbgn(hiPort);
  if (dclient.connect(pasvServer, hiPort, timeout))
  {
    FTPdbgn(F("Data connection established"));
  }
}

void ESP32_FTPClient::appendFile(char *fileName)
{
  FTPdbgn("Send APPE");
  if (!isConnected())
    return;
  client.print(F("APPE "));
  client.println(F(fileName));
  getFTPAnswer();
}

void ESP32_FTPClient::changeWorkDir(const char *dir)
{
  FTPdbgn("Send CWD");
  if (!isConnected())
    return;
  client.print(F("CWD "));
  client.println(F(dir));
  getFTPAnswer();
}

void ESP32_FTPClient::deleteFile(const char *file)
{
  FTPdbgn("Send DELE");
  if (!isConnected())
    return;
  client.print(F("DELE "));
  client.println(F(file));
  getFTPAnswer();
}

void ESP32_FTPClient::makeDir(const char *dir)
{
  FTPdbgn("Send MKD");
  if (!isConnected())
    return;
  client.print(F("MKD "));
  client.println(F(dir));
  getFTPAnswer();
}

void ESP32_FTPClient::contentList(const char *dir, String *list)
{
  char _resp[sizeof(outBuf)];
  uint16_t _b = 0;

  FTPdbgn("Send MLSD");
  if (!isConnected())
    return;
  client.print(F("MLSD "));
  client.println(F(dir));
  getFTPAnswer(_resp);

  // Convert char array to string to manipulate and find response size
  // each server reports it differently, TODO = FEAT
  //String resp_string = _resp;
  //resp_string.substring(resp_string.lastIndexOf('matches')-9);
  //FTPdbgn(resp_string);

  unsigned long _m = millis();
  while (!dclient.available() && millis() < _m + timeout)
    delay(1);

  while (dclient.available())
  {
    if (_b < 128)
    {
      list[_b] = dclient.readStringUntil('\n');
      //FTPdbgn(String(_b) + ":" + list[_b]);
      _b++;
    }
  }
}

void ESP32_FTPClient::contentListWithListCommand(const char *dir, String *list)
{
  char _resp[sizeof(outBuf)];
  uint16_t _b = 0;

  FTPdbgn("Send LIST");
  if (!isConnected())
    return;
  client.print(F("LIST "));
  client.println(F(dir));
  getFTPAnswer(_resp);

  // Convert char array to string to manipulate and find response size
  // each server reports it differently, TODO = FEAT
  //String resp_string = _resp;
  //resp_string.substring(resp_string.lastIndexOf('matches')-9);
  //FTPdbgn(resp_string);

  unsigned long _m = millis();
  while (!dclient.available() && millis() < _m + timeout)
    delay(1);

  while (dclient.available())
  {
    if (_b < 128)
    {
      String tmp = dclient.readStringUntil('\n');
      list[_b] = tmp.substring(tmp.lastIndexOf(" ") + 1, tmp.length());
      //FTPdbgn(String(_b) + ":" + tmp);
      _b++;
    }
  }
}

void ESP32_FTPClient::downloadString(const char *filename, String &str)
{
  FTPdbgn("Send RETR");
  if (!isConnected())
    return;
  client.print(F("RETR "));
  client.println(F(filename));

  char _resp[sizeof(outBuf)];
  getFTPAnswer(_resp);

  unsigned long _m = millis();
  while (!GetDataClient()->available() && millis() < _m + timeout)
    delay(1);

  while (GetDataClient()->available())
  {
    str += GetDataClient()->readString();
  }
}

void ESP32_FTPClient::downloadFile(const char *filename, unsigned char *buf, size_t length, bool printUART)
{
  FTPdbgn("Send RETR");
  if (!isConnected())
    return;
  client.print(F("RETR "));
  client.println(F(filename));

  char _resp[sizeof(outBuf)];
  getFTPAnswer(_resp);

  char _buf[2];

  unsigned long _m = millis();
  while (!dclient.available() && millis() < _m + timeout)
    delay(1);

  while (dclient.available())
  {
    if (!printUART)
      dclient.readBytes(buf, length);

    else
    {
      for (size_t _b = 0; _b < length; _b++)
      {
        dclient.readBytes(_buf, 1),
            Serial.print(_buf[0], HEX);
      }
    }
  }
}

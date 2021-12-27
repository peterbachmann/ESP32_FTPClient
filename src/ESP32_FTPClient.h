#ifndef _ESP32_FTPClient
#define _ESP32_FTPClient

#include <stdint.h>

class ESP32_FTPClient
{
private:
  void writeClientBuffered(WiFiClient *cli, unsigned char *data, int dataLength);
  char outBuf[128];
  unsigned char outCount;
  WiFiClient client;
  WiFiClient dclient;
  uint8_t verbose;

  template <typename T>
  void FTPdbg(T msg)
  {
    if (verbose == 2)
      Serial.print(msg);
  }

  template <typename T>
  void FTPdbgn(T msg)
  {
    if (verbose == 2)
      Serial.println(msg);
  }

  template <typename T>
  void FTPerr(T msg)
  {
    if (verbose == 1 || verbose == 2)
      Serial.print(msg);
  }

  char *userName;
  char *passWord;
  char *serverAdress;
  uint16_t port;
  bool _isConnected = false;
  unsigned char clientBuf[1500];
  size_t bufferSize = 1500;
  uint16_t timeout = 10000;
  WiFiClient *GetDataClient();

public:
  ESP32_FTPClient(char *_serverAdress, uint16_t _port, char *_userName, char *_passWord, uint16_t _timeout = 10000, uint8_t _verbose = 1);
  ESP32_FTPClient(char *_serverAdress, char *_userName, char *_passWord, uint16_t _timeout = 10000, uint8_t _verbose = 1);
  void openConnection();
  void closeConnection();
  bool isConnected();
  void newFile(const char *fileName);
  void appendFile(char *fileName);
  void writeData(unsigned char *data, int dataLength);
  void closeFile();
  void getFTPAnswer(char *result = NULL, int offsetStart = 0);
  void getLastModifiedTime(const char *fileName, char *result);
  void renameFile(char *from, char *to);
  void write(const char *str);
  void initFile(const char *type);
  void changeWorkDir(const char *dir);
  void deleteFile(const char *file);
  void makeDir(const char *dir);
  void contentList(const char *dir, String *list);
  void contentListWithListCommand(const char *dir, String *list);
  void downloadString(const char *filename, String &str);
  void downloadFile(const char *filename, unsigned char *buf, size_t length, bool printUART = false);
};

#endif
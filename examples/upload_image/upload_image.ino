/******************************************************************************

ESP32-CAM remote image access via FTP. Take pictures with ESP32 and upload it via FTP making it accessible for the outisde network. 
Leonardo Bispo
July - 2019
https://github.com/ldab/ESP32_FTPClient

Distributed as-is; no warranty is given.

******************************************************************************/
#include "Arduino.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <ESP32_FTPClient.h>
#include "octocat.h"

#define WIFI_SSID ""
#define WIFI_PASS ""

char ftp_server[] = "";
char ftp_user[] = "";
char ftp_pass[] = "";

// you can pass a FTP timeout and debbug mode on the last 2 arguments
ESP32_FTPClient ftp(ftp_server, ftp_user, ftp_pass, 5000, 2);

void setup()
{
  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.println("Connecting Wifi...");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  ftp.openConnection();

  // Get directory content
  ftp.initFile("Type A");
  String list[128];
  ftp.changeWorkDir("/public_html/zyro/gallery_gen/");
  ftp.contentList("", list);
  Serial.println("\nDirectory info: ");
  for (int i = 0; i < sizeof(list); i++)
  {
    if (list[i].length() > 0)
      Serial.println(list[i]);
    else
      break;
  }

  // Make a new directory
  ftp.initFile("Type A");
  ftp.makeDir("my_new_dir");

  // Create the new file and send the image
  ftp.changeWorkDir("my_new_dir");
  ftp.initFile("Type I");
  ftp.newFile("octocat.jpg");
  ftp.writeData(octocat_pic, sizeof(octocat_pic));
  ftp.closeFile();

  // Create the file new and write a string into it
  ftp.initFile("Type A");
  ftp.newFile("hello_world.txt");
  ftp.write("Hello World");
  ftp.closeFile();

  ftp.closeConnection();
}

void loop()
{
}
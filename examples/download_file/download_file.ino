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

#define WIFI_SSID ""
#define WIFI_PASS ""

char ftp_server[] = "files.000webhost.com";
char ftp_user[] = "";
char ftp_pass[] = "";

ESP32_FTPClient ftp(ftp_server, ftp_user, ftp_pass);

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

  Serial.print("\nMax Free Heap: ");
  Serial.println(ESP.getMaxAllocHeap());
  Serial.println("");

  ftp.openConnection();

  //Change directory
  ftp.changeWorkDir("/public_html/zyro/gallery_gen");

  // Create a new file to use as the download example below:
  ftp.initFile("Type A");
  ftp.newFile("helloworld.txt");
  ftp.write("Hi, I'm a new file");
  ftp.closeFile();

  //Download the text file or read it
  String response = "";
  ftp.initFile("Type A");
  ftp.downloadString("helloworld.txt", response);
  Serial.println("The file content is: " + response);

  // Get the file size
  const char *fileName = "myPhoto.png";
  size_t fileSize = 0;
  String list[128];

  // Get the directory content in order to allocate buffer
  // my server response is type=file;size=18;modify=20190731140703;unix.mode=0644;unix.uid=10183013;unix.gid=10183013;unique=809g7c8e92e4; helloworld.txt
  ftp.initFile("Type A");
  ftp.contentList("", list);
  for (uint8_t i = 0; i < sizeof(list); i++)
  {
    uint8_t indexSize = 0;
    uint8_t indexMod = 0;

    if (list[i].length() > 0)
    {
      list[i].toLowerCase();

      if (list[i].indexOf(fileName) > -1)
      {
        indexSize = list[i].indexOf("size") + 5;
        indexMod = list[i].indexOf("modify") - 1;

        fileSize = list[i].substring(indexSize, indexMod).toInt();
      }

      // Print the directory details
      Serial.println(list[i]);
    }
    else
      break;
  }

  // Print file size
  Serial.println("\nFile size is: " + String(fileSize));

  //Dynammically alocate buffer
  unsigned char *downloaded_file = (unsigned char *)malloc(fileSize);

  // And download the file
  ftp.initFile("Type I");
  ftp.downloadFile(fileName, downloaded_file, fileSize, false);

  //Create a new Directory
  ftp.initFile("Type I");
  ftp.makeDir("myNewDir");

  //Enter the directory
  ftp.changeWorkDir("/public_html/zyro/gallery_gen/myNewDir");

  //And upload the file to the new directory
  ftp.newFile(fileName);
  ftp.writeData(downloaded_file, fileSize);
  ftp.closeFile();

  free(downloaded_file);

  ftp.closeConnection();
}

void loop()
{
}
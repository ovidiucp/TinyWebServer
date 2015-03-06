// -*- c++ -*-
//
// Copyright 2010 Ovidiu Predescu <ovidiu@gmail.com>
// Date: June 2010
// Updated: 08-JAN-2012 for Arduno IDE 1.0 by <Hardcore@hardcoreforensics.com>
//

#include <SPI.h>
#include <Ethernet.h>
#include <Flash.h>
#include <SD.h>
#include <TinyWebServer.h>

boolean file_handler(TinyWebServer& web_server);
boolean index_handler(TinyWebServer& web_server);

boolean has_filesystem = true;
Sd2Card card;
SdVolume volume;
SdFile root;
SdFile file;

static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Don't forget to modify the IP to an available one on your home network
byte ip[] = { 192, 168, 178, 177 };

const int sdChipSelect = 4;            // SD card chipSelect

TinyWebServer::PathHandler handlers[] = {
  {"/", TinyWebServer::GET, &index_handler },
  {"/" "*", TinyWebServer::GET, &file_handler },
  {NULL},
};

boolean file_handler(TinyWebServer& web_server) {

  if(!has_filesystem) {
    web_server.send_error_code(500);
    web_server << F("Internal Server Error");
    return true;
  }

  char* filename = TinyWebServer::get_file_from_path(web_server.get_path());

  if(!filename) {
  	web_server.send_error_code(400);
  	web_server << F("Bad Request");
  	return true;
  }

  send_file_name(web_server, filename);
  free(filename);
  return true;
}

void send_file_name(TinyWebServer& web_server, const char* filename) {

  TinyWebServer::MimeType mime_type
    = TinyWebServer::get_mime_type_from_filename(filename);
  if (file.open(&root, filename, O_READ)) {
    web_server.send_error_code(200);
    web_server.send_content_type(mime_type);
    web_server.end_headers();

    Serial << F("Read file "); Serial.println(filename);
    web_server.send_file(file);
    file.close();
  } else {
    web_server.send_error_code(404);
    web_server.send_content_type("text/plain");
    web_server.end_headers();

    Serial << F("Could not find file: "); Serial.println(filename);
    web_server << F("404 - File not found") << filename << "\n";
  }
}

boolean index_handler(TinyWebServer& web_server) {
  web_server.send_error_code(200);
  web_server.end_headers();
  web_server << F("<html><body><h1>Hello World!</h1></body></html>\n");
  return true;
}

boolean has_ip_address = false;
TinyWebServer web = TinyWebServer(handlers, NULL);

const char* ip_to_str(const uint8_t* ipAddr)
{
  static char buf[16];
  sprintf(buf, "%d.%d.%d.%d\0", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
  return buf;
}

void setup() {
  Serial.begin(9600);

  Serial << F("Free RAM: ") << FreeRam() << "\n";

  pinMode(10, OUTPUT); // set the SS pin as an output (necessary!)
  digitalWrite(10, HIGH); // but turn off the W5100 chip!
  // initialize the SD card

  Serial << F("Setting up SD card...\n");

  if (!card.init(SPI_FULL_SPEED, 4)) {
    Serial << F("card failed\n");
    has_filesystem = false;
  }
  // initialize a FAT volume
  if (!volume.init(&card)) {
    Serial << F("vol.init failed!\n");
    has_filesystem = false;
  }
  if (!root.openRoot(&volume)) {
    Serial << F("openRoot failed");
    has_filesystem = false;
  }

  Serial << F("Setting up the Ethernet card...\n");
  Ethernet.begin(mac, ip);

  // Start the web server.
  Serial << F("Web server starting...\n");
  web.begin();

  Serial << F("Free RAM: ") << FreeRam() << "\n";

  Serial << F("Ready to accept HTTP requests.\n\n");

}

void loop() {
  web.process();
}

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

boolean index_handler(TinyWebServer& web_server);

static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

// Don't forget to modify the IP to an available one on your home network
byte ip[] = { 192, 168, 5, 177 };

TinyWebServer::PathHandler handlers[] = {
  {"/", TinyWebServer::GET, &index_handler },
  {NULL},
};

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
  Serial.begin(115200);

  pinMode(10, OUTPUT); // set the SS pin as an output (necessary!)
  digitalWrite(10, HIGH); // but turn off the W5100 chip!
  pinMode(4, OUTPUT); // set the SS pin as an output (necessary!)
  digitalWrite(4, HIGH); // but turn off the SD card functionality!

  Serial << F("Free RAM: ") << FreeRam() << "\n";

  Serial << F("Setting up the Ethernet card...\n");
  Ethernet.begin(mac, ip);

  // Start the web server.
  Serial << F("Web server starting...\n");
  web.begin();

  Serial << F("Ready to accept HTTP requests.\n\n");
}

void loop() {
  web.process();
}
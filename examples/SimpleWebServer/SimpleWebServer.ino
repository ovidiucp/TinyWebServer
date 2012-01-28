// -*- c++ -*-
//
// Copyright 2010 Ovidiu Predescu <ovidiu@gmail.com>
// Date: June 2010
// Updated: 08-JAN-2012 for Arduno IDE 1.0 by <Hardcore@hardcoreforensics.com>
//

#include <pins_arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <Flash.h>
#include <SD.h>
#include <TinyWebServer.h>

/****************VALUES YOU CHANGE*************/
// pin 4 is the SPI select pin for the SDcard
const int SD_CS = 4;

// pin 10 is the SPI select pin for the Ethernet
const int ETHER_CS = 10;

// Don't forget to modify the IP to an available one on your home network
byte ip[] = { 192, 168, 5, 177 };
/*********************************************/

static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

boolean index_handler(TinyWebServer& web_server);

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
  Serial << F("Free RAM: ") << FreeRam() << "\n";

  pinMode(SS_PIN, OUTPUT);	// set the SS pin as an output
                                // (necessary to keep the board as
                                // master and not SPI slave)
  digitalWrite(SS_PIN, HIGH);	// and ensure SS is high

  // Ensure we are in a consistent state after power-up or a reset
  // button These pins are standard for the Arduino w5100 Rev 3
  // ethernet board They may need to be re-jigged for different boards
  pinMode(ETHER_CS, OUTPUT);	// Set the CS pin as an output
  digitalWrite(ETHER_CS, HIGH);	// Turn off the W5100 chip! (wait for
                                // configuration)
  pinMode(SD_CS, OUTPUT);	// Set the SDcard CS pin as an output
  digitalWrite(SD_CS, HIGH);	// Turn off the SD card! (wait for
                                // configuration)

  // Initialize the Ethernet.
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

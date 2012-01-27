// -*- c++ -*-
//
// Copyright 2010 Ovidiu Predescu <ovidiu@gmail.com>
// Date: June 2010
// Updated: 08-JAN-2012 for Arduno IDE 1.0 by <Hardcore@hardcoreforensics.com>
//

extern "C" {
#include <ctype.h>
}

#include <Ethernet.h>
#include <SPI.h>
#include <Flash.h>
#include <SD.h>

#include <TinyWebServer.h>

class TinyWebServerTest : public TinyWebServer {
public:
  TinyWebServerTest(PathHandler handlers[], const char** headers,
		    const _FLASH_STRING& content)
    : TinyWebServer(handlers, headers),
      content_(content),
      pos_(0) {}

  static char* get_field_public(const char* buffer, int which) {
    return get_field(buffer, which);
  }

protected:
  const _FLASH_STRING& content_;
  uint16_t pos_;

  // Returns true if the HTTP request processing should be stopped.
  virtual boolean should_stop_processing() { return false; }

  // Reads a character from the request's input stream. Returns true
  // if the character could be read, false otherwise.
  virtual boolean read_next_char(Client& client, uint8_t* ch) {
    if (pos_ == content_.length()) {
      return false;
    } else {
      *ch = content_[pos_++];
      return true;
    }
  }
};

int failures = 0;

void expect_str_eq(const char* s1, char* s2, boolean free_s2 = true) {
  if (strcmp(s1, s2) != 0) {
    Serial << F("FAIL: expect ") << s1 << F(", got ") << s2 << "\n";
    failures++;
  }
  if (free_s2) {
    free(s2);
  }
}

void expect_num_eq(uint32_t n1, uint32_t n2) {
  if (n1 != n2) {
    Serial << F("FAIL: expect ") << n1 << F(", got ") << n2 << "\n";
    failures++;
  }
}

void expect_true(boolean r) {
  if (!r) {
    Serial << F("FAIL: expected true, got false\n");
    failures++;
  }
}

void test_decode_url_encoded() {
  expect_str_eq("index.htm", TinyWebServer::decode_url_encoded("index%2Ehtm"));
  expect_str_eq("index.", TinyWebServer::decode_url_encoded("index%2E"));
  expect_str_eq("index%2", TinyWebServer::decode_url_encoded("index%2"));
  expect_str_eq(".index", TinyWebServer::decode_url_encoded("%2Eindex"));
  expect_str_eq("", TinyWebServer::decode_url_encoded(""));

  // Invalid escape sequences
  expect_str_eq("%GEhtm", TinyWebServer::decode_url_encoded("%GEhtm"));
  expect_str_eq("%%Ehtm", TinyWebServer::decode_url_encoded("%%Ehtm"));
}

void test_get_file_from_path() {
  expect_str_eq("", TinyWebServer::get_file_from_path("/"));
  expect_str_eq("INDEX.HTM", TinyWebServer::get_file_from_path("/index%2Ehtm"));
  expect_str_eq("INDEX.HTM",
  		TinyWebServer::get_file_from_path("/upload/index%2Ehtm"));
  expect_str_eq("INDEX.HTM",
  		TinyWebServer::get_file_from_path("/a/b/index%2Ehtm"));
}

void test_get_mime_type_from_filename() {
  uint16_t codes[9];
  uint16_t html_code;

  codes[0] = TinyWebServer::get_mime_type_from_filename("index.htm");
  html_code = TinyWebServer::get_mime_type_from_filename("INDEX.HTM");
  expect_num_eq(codes[0], html_code);

  codes[1] = TinyWebServer::get_mime_type_from_filename("file.txt");
  html_code = TinyWebServer::get_mime_type_from_filename("FILE.TXT");
  expect_num_eq(codes[1], html_code);

  codes[2] = TinyWebServer::get_mime_type_from_filename("style.css");
  html_code = TinyWebServer::get_mime_type_from_filename("STYLE.CSS");
  expect_num_eq(codes[2], html_code);

  codes[3] = TinyWebServer::get_mime_type_from_filename("file.xml");
  html_code = TinyWebServer::get_mime_type_from_filename("FILE.XML");
  expect_num_eq(codes[3], html_code);

  codes[4] = TinyWebServer::get_mime_type_from_filename("img.gif");
  html_code = TinyWebServer::get_mime_type_from_filename("IMG.GIF");
  expect_num_eq(codes[4], html_code);

  codes[5] = TinyWebServer::get_mime_type_from_filename("img.jpg");
  html_code = TinyWebServer::get_mime_type_from_filename("IMG.JPG");
  expect_num_eq(codes[5], html_code);

  codes[6] = TinyWebServer::get_mime_type_from_filename("img.png");
  html_code = TinyWebServer::get_mime_type_from_filename("IMG.PNG");
  expect_num_eq(codes[6], html_code);

  codes[7] = TinyWebServer::get_mime_type_from_filename("img.ico");
  html_code = TinyWebServer::get_mime_type_from_filename("IMG.ICO");
  expect_num_eq(codes[7], html_code);

  codes[8] = TinyWebServer::get_mime_type_from_filename("p.mp3");
  html_code = TinyWebServer::get_mime_type_from_filename("P.MP3");
  expect_num_eq(codes[8], html_code);

  for (int i = 0; i < 8; i++) {
    if (codes[i] >= codes[i + 1]) {
      Serial << F("FAIL: expect ") << codes[i] << ", got " << codes[i+1] <<"\n";
      failures++;
    }
  }
}

void test_get_field() {
  {
    char b[] = "GET / HTTP/1.0";
    expect_str_eq("GET", TinyWebServerTest::get_field_public(b, 0));
    expect_str_eq("/", TinyWebServerTest::get_field_public(b, 1));
    expect_str_eq("HTTP/1.0",
                  TinyWebServerTest::get_field_public(b, 2));
  }

  {
    char b[] = "  GET  /  HTTP/1.0  ";
    expect_str_eq("GET", TinyWebServerTest::get_field_public(b, 0));
    expect_str_eq("/", TinyWebServerTest::get_field_public(b, 1));
    expect_str_eq("HTTP/1.0", TinyWebServerTest::get_field_public(b, 2));
  }

  {
    char b[] = "GET / HTTP/1.0";
    expect_str_eq(NULL, TinyWebServerTest::get_field_public(b, 3));
  }
}

void test_process_headers() {
  FLASH_STRING(content,
	       "User-Agent: curl/7.19.7\r\n"
	       "Host: arduino\r\n"
	       "Accept: */" "*\r\n"
	       "Content-Length: 49209\r\n"
	       "\r\n"
	       );
  const char* headers[] = {
    "Content-Length",
    NULL
  };

  TinyWebServerTest web(NULL, headers, content);
  expect_true(web.process_headers());
  expect_str_eq("49209", (char*)web.get_header_value("Content-Length"),
		false /* don't free the second argument */);
  expect_str_eq(NULL, (char*)web.get_header_value("Host"),
		false /* don't free the second argument */);
  expect_str_eq(NULL, (char*)web.get_header_value("random-header"),
		false /* don't free the second argument */);
}

void test_process_broken_headers() {
  FLASH_STRING(content,
	       "User-Agent curl/7.19.7\r\n"
	       );

  TinyWebServerTest web(NULL, NULL, content);
  expect_true(!web.process_headers());
}

void setup() {
  Serial.begin(115200);
  Serial << F("Free RAM: ") << FreeRam() << "\n";

  test_decode_url_encoded();
  test_get_file_from_path();
  test_get_mime_type_from_filename();
  test_get_field();
  test_process_headers();
  test_process_broken_headers();

  if (!failures) {
    Serial << F("\nSUCCESS\n");
  }
}

void loop() {
}

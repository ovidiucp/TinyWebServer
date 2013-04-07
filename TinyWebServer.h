// -*- c++ -*-
//
// Copyright 2010 Ovidiu Predescu <ovidiu@gmail.com>
// Date: May, June 2010
//
// Updated: 08-JAN-2012 for Arduno IDE 1.0 by <Hardcore@hardcoreforensics.com>
//
// TinyWebServer for Arduino.

#ifndef __WEB_SERVER_H__
#define __WEB_SERVER_H__

#include <Print.h>

class SdFile;
class TinyWebServer;

namespace TinyWebPutHandler {
  enum PutAction {
    START,
    WRITE,
    END
  };

  typedef void (*HandlerFn)(TinyWebServer& web_server,
			    PutAction action,
			    char* buffer, int size);

  // An HTTP handler that knows how to handle file uploads using the
  // PUT method. Set the `put_handler_fn' variable below to your own
  // function to handle the characters of the uploaded function.
  boolean put_handler(TinyWebServer& web_server);
  extern HandlerFn put_handler_fn;
};

class TinyWebServer : public Print {
public:
  // An HTTP path handler. The handler function takes the path it
  // registered for as argument, and the Client object to handle the
  // response.
  //
  // The function should return true if it finished handling the request
  // and the connection should be closed.
  typedef boolean (*WebHandlerFn)(TinyWebServer& web_server);

  enum HttpRequestType {
    UNKNOWN_REQUEST,
    GET,
    HEAD,
    POST,
    PUT,
    DELETE,
    ANY,
  };

  // An identifier for a MIME type. The number is opaque to a human,
  // but it's really an offset in the `mime_types' array.
  typedef uint16_t MimeType;

  typedef struct {
    const char* path;
    HttpRequestType type;
    WebHandlerFn handler;
  } PathHandler;

  // Initialize the web server using a NULL terminated array of path
  // handlers, and a NULL terminated array of headers the handlers are
  // interested in.
  //
  // NOTE: Make sure the header names are all lowercase.
  TinyWebServer(PathHandler handlers[], const char** headers,
                const int port=80);

  // Call this method to start the HTTP server
  void begin();

  // Handles a possible HTTP request. It will return immediately if no
  // client has connected. Otherwise the request is handled
  // synchronously.
  //
  // Call this method from the main loop() function to have the Web
  // server handle incoming requests.
  void process();

  // Sends the HTTP status code to the connect HTTP client.
  void send_error_code(int code) {
    send_error_code(client_, code);
  }
  static void send_error_code(Client& client, int code);

  void send_content_type(MimeType mime_type);
  void send_content_type(const char* content_type);

  // Call this method to indicate the end of the headers.
  inline void end_headers() { client_.println(); }
  static inline void end_headers(Client& client) { client.println(); }

  // void send_error_code(MimeType mime_type, int code);
  // void send_error_code(const char* content_type, int code);

  const char* get_path();
  const HttpRequestType get_type();
  const char* get_header_value(const char* header);
  EthernetClient& get_client() { return client_; }

  // Processes the HTTP headers and assigns values to the requested
  // ones in headers_. Returns true when successful, false in case of
  // errors.
  boolean process_headers();

  // Helper methods

  // Assumes `s' is an HTTP encoded URL, replaces all the escape
  // characters in it and returns the unencoded version. For example
  // for "/index%2Ehtm", this method returns "index.htm".
  //
  // The returned string must be free()d by the caller.
  static char* decode_url_encoded(const char* s);

  // Assumes the last component of the URL path is a file
  // name. Returns the file name in upper case, ready to passed to
  // SdFile's open() method.
  //
  // In addition to the file name, it sets `mime_type' to an identifier
  //
  // The returned string must be free()d by the caller.
  static char* get_file_from_path(const char* path);

  // Guesses a MIME type based on the extension of `filename'. If none
  // could be guessed, the equivalent of text/html is returned.
  static MimeType get_mime_type_from_filename(const char* filename);

  // Sends the contents of `file' to the currently connected
  // client. The file must be opened in read mode.
  //
  // This is mainly an optimization to reuse the internal static
  // buffer used by this class, which saves us some RAM.
  void send_file(SdFile& file);

  // These methods write directly in the response stream of the
  // connected client
  virtual size_t write(uint8_t c);
  virtual size_t write(const char *str);
  virtual size_t write(const uint8_t *buffer, size_t size);

  // Some methods used for testing purposes

  // Returns true if the HTTP request processing should be stopped.
  virtual boolean should_stop_processing() { return !client_.connected();}

  // Reads a character from the request's input stream. Returns true
  // if the character could be read, false otherwise.
  virtual boolean read_next_char(Client& client, uint8_t* ch);

 protected:
  // Returns the field number `which' from buffer. Fields are
  // separated by spaces. Should be a private method, but made public
  // so it can be tested.
  static char* get_field(const char* buffer, int which);

private:
  // The path handlers
  PathHandler* handlers_;

  typedef struct {
    const char* header;
    char* value;
  } HeaderValue;

  // The headers
  HeaderValue* headers_;

  // The TCP/IP server we use.
  EthernetServer server_;

  char* path_;
  HttpRequestType request_type_;
  EthernetClient client_;

  // Reads a line from the HTTP request sent by an HTTP client. The
  // line is put in `buffer' and up to `size' characters are written
  // in it.
  boolean get_line(char* buffer, int size);

  // Returns true if the header is marked as requested in the headers_
  // array. As a side effect, the pointer to the actual header is made
  // to point to the one in the headers_ array.
  boolean is_requested_header(const char** header);

  boolean assign_header_value(const char* header, char* value);
};

#endif /* __WEB_SERVER_H__ */

Date: 2015-03-06 14:58:18
Arduino TinyWebServer

A small web server for Arduino.

Check out this video demonstrating what you can do with it:

http://www.youtube.com/watch?v=qZXKk6nCYuM

You can read up on TinyWebServer's history here:

http://www.webweavertech.com/ovidiu/weblog/archives/000484.html

http://www.webweavertech.com/ovidiu/weblog/archives/000477.html

The library is licensed under the terms of LGPL 2.1. Which means
you're free to use it in your projects (including commercial ones) as
long as you're sending back the changes you make to the library.

External dependencies
====================

TinyWebServer depends on the external library Flash version 5.0, which
is found here:

http://arduiniana.org/libraries/flash/

Make sure you dowload the Flash library and install it in your
Arduino's `libraries` directory, as described in this document:

http://arduino.cc/en/Guide/Libraries

If you're using a version of Arduino IDE newer than 1.5, you need to
modify the `Flash.h` file to include the following lines just after
the `#include <avr/pgmspace.h>` line:

```
#if ARDUINO >= 150
typedef char prog_char __attribute__((__progmem__));
#endif
```

If you're using an ARM Cortex M0-based board, such as the Arduino Zero
or the MKR1000, you need to modify the `Flash.h` file to include the
following:

```
#if ARDUINO_ARCH_SAMD
extern char* strncpy_P(char* dest, const char* src, int size);
#endif
```

Basic web server
================

To make use of the TinyWebServer library, you need to include the
following your sketch:

    #include <Ethernet.h>
    #include <Flash.h>
    #include <SD.h>
    #include <TinyWebServer.h>

TWS is implemented by the TinyWebServer class. The constructor method
takes two arguments. The first one is a list of handlers, functions to
be invoked when a particular URL is requested by an HTTP client. The
second one is a list of HTTP header names that are needed by the
implementation of your handlers. More on these later.

An HTTP handler is a simple function that takes as argument a
reference to the TinyWebServer object. When you create the
TinyWebServer class, you need to pass in the handlers for the various
URLs. Here is a simple example of a web server with a single handler.

    static uint8_t mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

    boolean index_handler(TinyWebServer& web_server) {
      web_server.send_error_code(200);
      web_server << F("<html><body><h1>Hello World!</h1></body></html>\n");
      return true;
    }

    TinyWebServer::PathHandler handlers[] = {
      // Register the index_handler for GET requests on /
      {"/", TinyWebServer::GET, &index_handler },
      {NULL}, // The array has to be NULL terminated this way
    };

    // Create an instance of the web server. No HTTP headers are requested
    // by the HTTP request handlers.
    TinyWebServer web = TinyWebServer(handlers, NULL);

    void setup() {
      Serial.begin(115200);
      EthernetDHCP.begin(mac);
      web.begin();
    }

    void loop() {
      EthernetDHCP.maintain();
      web.process();
    }

In the loop() function we need the call to the process() to make sure
HTTP requests are serviced. If there is no new request, the method
returns immediately. Otherwise the process() method blocks until the
request is handled.

For a complete working example look in
TinyWebServer/example/SimpleWebServer.

Serving files from the SD card
==============================

Now that we've seen the basics, let's see how we can extend this web
server to serve files stored on the SD card. The idea is to register a
handler that serves any URLs. Once the handler is invoked, it
interprets the URL path as a file name on the SD card and returns
that.

    boolean file_handler(TinyWebServer& web_server) {
      char* filename = TinyWebServer::get_file_from_path(web_server.get_path());
      if (!filename) {
        web_server.send_error_code(404);
        web_server << "Could not parse URL";
      } else {
        TinyWebServer::MimeType mime_type
          = TinyWebServer::get_mime_type_from_filename(filename);
        web_server.send_error_code(mime_type, 200);
        if (file.open(filename, O_READ)) {
          web_server.send_file(file);
          file.close();
        } else {
          web_server << "Could not find file: " << filename << "\n";
        }
        free(filename);
      }
      return true;
    }

We can now register this in the handlers array:

    TinyWebServer::PathHandler handlers[] = {
      {"/" "*", TinyWebServer::GET, &file_handler },
      {NULL},
    };

Note how the URL for the HTTP request is specified. We want it to
be /*, very much like a regular expression. However Arduino's IDE
preprocessor has a bug in how it handles /* inside strings. By
specifying the string as "/" "*" we avoid the bug, while letting the
compiler optimize and concatenate the two strings into a single one.

The * works only at the end of a URL, anywhere else it would be
interpreted as part of the URL. If the * is at the end of the URL, the
code in TinyWebServer assumes the handler can process requests that
match the URL prefix. For example, if the URL string was /html/* then
any URL starting with /html/ would be handled by the specified
handler. In our case, since we specified /*, any URL starting with /
(except for the top level / URL) will invoke the specified handler.

Uploading files to the web server and store them on SD card's file system
=========================================================================

Now wouldn't it be nice to update Arduino's Web server files using
HTTP? This way we can focus on building the actual interface with the
hardware, and provide just enough HTTP handlers to interact with
it. After we implement a minimal user interface, we can iterate it
without having to remove the SD card from the embedded project, copy
the HTML, JavaScript and/or image files on a computer, and plug it
back in. We could do this remotely from the computer, using a simple
script.

TinyWebServer provides a simple file upload HTTP handler that uses the
HTTP 1.0 PUT method. This allows you to implement an Ajax interface
using XMLHttpRequest or simply use a tool like curl to implement file
uploads.

Here's how you add file uploads to your Arduino web server:

    TinyWebServer::PathHandler handlers[] = {
      // `put_handler' is defined in TinyWebServer
      {"/upload/" "*", TinyWebServer::PUT, &TinyWebPutHandler::put_handler },
      {"/" "*", TinyWebServer::GET, &file_handler },
      {NULL},

Note that the order in which you declare the handlers is
important. The URLs are matched in the order in which they are
declared.

This is where the headers array mentioned before comes into
picture. The put_handler makes use of the Content-Length. To avoid
unnecessary work and minimize precious memory usage, TinyWebServer
does not do any header processing unless it's instructed. To do so,
you need to declare an array of header names your handlers are
interested in. In this case, we need to add Content-Length.

    const char* headers[] = {
      "Content-Length",
      NULL
    };

And we now initialize the instance of TinyWebServer like this:

    TinyWebServer web = TinyWebServer(handlers, headers);

The put_handler method is really generic, it doesn't actually
implement the code to write the file to disk. Instead the method
relies on a user provided function that implements the actual
logic. This allows you to use a different file system implementation
than Fat16 or do something totally different than write the file to
disk.

The user provided function take 4 parameters. The first is a reference
to the TinyWebServer instance. The second is a PutAction enum which
could be either START, WRITE or END. START and END are called exactly
once during a PUT handler's execution, while WRITE is called multiple
times. Each time the function is called with the WRITE param, the
third and fourth parameters are set to a buffer and a number of bytes
in this buffer that should be used.

Here is a small example of a user provided function that writes the
PUT request's content to a file:

    void file_uploader_handler(TinyWebServer& web_server,
                               TinyWebPutHandler::PutAction action,
                               char* buffer, int size) {
      static uint32_t start_time;

      switch (action) {
      case TinyWebPutHandler::START:
        start_time = millis();
        if (!file.isOpen()) {
          // File is not opened, create it. First obtain the desired name
          // from the request path.
          char* fname = web_server.get_file_from_path(web_server.get_path());
          if (fname) {
            Serial << "Creating " << fname << "\n";
            file.open(fname, O_CREAT | O_WRITE | O_TRUNC);
            free(fname);
          }
        }
        break;

      case TinyWebPutHandler::WRITE:
        if (file.isOpen()) {
          file.write(buffer, size);
        }
        break;

      case TinyWebPutHandler::END:
        file.sync();
        Serial << "Wrote " << file.fileSize() << " bytes in "
               << millis() - start_time << " millis\n";
        file.close();
      }
    }

To activate this user provided function, assign its address to
put_handler_fn, like this:

    void setup() {
      // ...

      // Assign our function to `upload_handler_fn'.
      TinyWebPutHandler::put_handler_fn = file_uploader_handler;

      // ...
    }

You can now test uploading a file using curl:

*Note that since the handler in the source looks like this:

{"/upload/" "*", TinyWebServer::PUT, &TinyWebPutHandler::put_handler }

you must ensure that the path '/upload/' is in your submitted URL


    curl -0 -T index.htm http://my-arduino-ip-address/upload/

For a complete working example of the file upload and serving web
server, look in TinyWebServer/examples/FileUpload.

Advanced topic: persistent HTTP connections
===========================================

Sometimes it's useful to have an HTTP client start a request. For
example, I need to be able to enter an IR learning process. This means
that I cannot afford TinyWebServer's process() to block while serving
my /learn request that initiated the IR learning process. Instead I
want the handler of the /learn request to set a variable in the code
that indicates that IR learning is active, and then return
immediately.

If you noticed the HTTP handlers return a boolean. If the returned
value is true, as it was the case in our examples above, the
connection to the HTTP client is closed immediately. If the returned
value is false the connection is left open. Your handler should save
the Client object handling the HTTP connection with the original
request. Your code becomes responsible with closing it when it's no
longer needed.

To obtain the Client object, use the get_client() method while in the
HTTP handler. You can write asynchronously to the client, to update it
with the state of the web server.

In my remotely controlled projection screen application, I have
another handler on /cancel that closes the /learn client
forcibly. Otherwise the /learn's Client connection is closed at the
end of the IR learning procedure. Since the Ethernet shield only
allows for 4 maximum HTTP clients open at the same time (because of 4
maximum client sockets), in my application I allow only one /learn
handler to be active at any given time.

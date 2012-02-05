To get this example running, make sure you modify the IP address
inside the FileUpload.ino and upload.sh files to match an unused IP
address in your network.

Then run the upload.sh script to upload the files in the static
directory to your Arduino web server. Point a browser to the IP
address you specified above.

If you're on Windows, make sure you install curl first on your
machine. Then open a terminal and run curl like below for each of the
files in the static/ directory. Replace $ARDUINO with the IP address
of your Arduino and $file with the name of the file you want to
upload.

curl -0 -T $file http://$ARDUINO/upload/

*Note that since the handler in the 'FileUpload.ino' looks like this:

{"/upload/" "*", TinyWebServer::PUT, &TinyWebPutHandler::put_handler }

you must ensure that the path '/upload/' is in your submitted URL


If you're a Windows developer, I'd appreciate if you could write a
small batch file that does the equivalent of update.sh.

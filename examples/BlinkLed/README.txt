To get this example running, make sure you modify the IP address
inside the BlinkLed.pde and upload.sh files to match an unused IP
address in your network.

Connect an LED with a resistor to pin 7 of your Arduino (the +
terminal of the LED should be on Pin 7). Connect LED's GND to a ground
pin on your Arduino. See the video below for how it works:

http://www.youtube.com/watch?v=qZXKk6nCYuM

Then run the upload.sh script to upload the files in the static
directory to your Arduino web server. Point a browser to the IP
address you specified above.

If you're on Windows, make sure you install curl first on your
machine. Then open a terminal and run curl like below for each of the
files in the static/ directory. Replace $ARDUINO with the IP address
of your Arduino and $file with the name of the file you want to
upload.

curl -0 -T $file http://$ARDUINO/upload/

If you're a Windows developer, I'd appreciate if you could write a
small batch file that does the equivalent of update.sh.

About
=====
The images were obtained from:

http://www.clker.com/clipart-light-bulb-led-off.html
http://www.clker.com/clipart-light-bulb-led-on.html

Prepared in Photoshop by me, then sprited using:

http://spritegen.website-performance.org/


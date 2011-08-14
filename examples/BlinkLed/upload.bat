set IPADDRESS=http://192.168.0.88
rem Be sure to change IP address (above) to match your board.  No other modification to this file should be needed
@echo off

rem Run this bat file to load web pages to the SD card on your Arduino Ethernet shield.
rem This file is only needed with windows - otherwise use upload.sh

set SubDir=static/
for %%f in (%SubDir%*.*) do (
echo uploading %SubDir%%%f to %IPADDRESS%/upload/
curl.exe -0 -T %SubDir%%%f %IPADDRESS%/upload/

)
echo Done!
pause

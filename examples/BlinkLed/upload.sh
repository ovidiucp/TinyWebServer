#! /bin/sh

# Copyright 2010 Ovidiu Predescu <ovidiu@gmail.com>
# Date: June 2010

ARDUINO=192.42.172.237

if [ $# != 0 ]; then
    FILES="$*"
else
    FILES=static/*
fi

for f in $FILES; do
    if [[ $(echo $f | egrep "~|CVS") ]]; then
	echo Skipping $f
    else
	size=`ls -l $f | awk '{print $5}'`
	echo "Uploading $f ($size bytes)"
        curl -0 -T $f http://$ARDUINO/upload/
    fi
done

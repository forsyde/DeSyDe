#!/bin/bash


for i in $( ls -d DATE_*5); do
    echo dir: $i
    #rm -rf $i/out/
    #rm $i/desConst.csv
    #rm $i/wcet.csv
    #mkdir -p $i/xmls
    cp ./platform.xml $i/xmls/platform.xml
    #cp ./desConst.xml $i/xmls/desConst.xml
    #move all sdf graph files into an sdfs directory
    #for j in $( ls $i/*.*sdf.xml); do
    #    FILENAME=$(echo $j| cut -d'/' -f 2)
    #    echo file: $FILENAME
    #    mv $j $i/sdfs
    #done
done



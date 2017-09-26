#!/bin/bash


for i in $( ls -d exp_*); do
    echo dir: $i
    DIRNAME=$(echo $i| cut -d'/' -f 1)
    rm $i/procData.csv
    #cp exp_3_1_rajp/config.cfg $DIRNAME/config.cfg
    #sed -i -e "s:exp_3_1_rajp/:$DIRNAME/:g" $DIRNAME/config.cfg
    #sed -i -e 's:criteria=THROUGHPUT:criteria=THROUGHPUT'"\n"'criteria=POWER:g' $DIRNAME/config.cfg
    #rm -rf $i/out/
    #rm $i/desConst.csv
    #rm $i/wcet.csv
    #mkdir -p $i/xmls
    #cp ./platform.xml $i/xmls/platform.xml
    #cp ./desConst.xml $i/xmls/desConst.xml
    #move all sdf graph files into an sdfs directory
    #for j in $( ls $i/*.*sdf.xml); do
    #    FILENAME=$(echo $j| cut -d'/' -f 2)
    #    echo file: $FILENAME
    #    mv $j $i/sdfs
    #done
done



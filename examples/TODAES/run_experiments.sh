#!/bin/bash
Experiment="--- Experiments for TODAES 2018 ---"
echo $Experiment
echo $Experiment > log.txt

echo -n "Insert the experiment number (1-6) > "
read config
IFS=$' ' read exp_no <<< "$config"
until [ "${exp_no:-0}" -gt 0 ] && [ "${exp_no:-0}" -lt 7 ]; do
    echo $config " is not a number between 1 and 6. Try again..."
    echo -n "Insert the experiment number (1-6) > "
    read config
    IFS=$' ' read exp_no <<< "$config"
done
echo "Running Experiment "$exp_no
printf "\n" >> log.txt
echo "Running Experiment "$exp_no >> log.txt



if [ $exp_no -eq 1 ] || [ $exp_no -eq 2 ]; then
    mkdir -p -v exp_$exp_no/out/
    date >> log.txt
    ls exp_$exp_no/sdfs >> log.txt
    SECONDS=0
    ./../../bin/adse --config exp_$exp_no/config.cfg --dse.th_prop MCR
    duration=$SECONDS
    echo "$(($duration / 60)) minutes and $(($duration % 60)) seconds elapsed." >> log.txt
    echo "end of experiment "$i >> log.txt
    date >> log.txt
fi

if [ $exp_no -eq 3 ] || [ $exp_no -eq 5 ]; then
    firstIndex=1 
    lastIndex=6
    for (( i=$firstIndex;i<=$lastIndex;i++)); do
        expName=$(ls -d exp_"$exp_no"_"$i"_*)
        echo $expName
        mkdir -p -v $expName/out/
        date >> log.txt
        ls $expName/sdfs >> log.txt
        SECONDS=0
        ./../../bin/adse --config $expName/config.cfg --dse.th_prop MCR
        duration=$SECONDS
        echo "$(($duration / 60)) minutes and $(($duration % 60)) seconds elapsed." >> log.txt
        echo "end of experiment "$i >> log.txt
        date >> log.txt
        done
fi
if [ $exp_no -eq 4 ] || [ $exp_no -eq 6 ]; then
    firstIndex=1 
    lastIndex=5
    for (( i=$firstIndex;i<=$lastIndex;i++)); do
        expName=$(ls -d exp_"$exp_no"_"$i"_*)
        echo $expName
        mkdir -p -v $expName/out/
        date >> log.txt
        ls $expName/sdfs >> log.txt
        SECONDS=0
        ./../../bin/adse --config $expName/config.cfg --dse.th_prop MCR
        duration=$SECONDS
        echo "$(($duration / 60)) minutes and $(($duration % 60)) seconds elapsed." >> log.txt
        echo "end of experiment "$i >> log.txt
        date >> log.txt
        done
fi
exit 

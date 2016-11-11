#!/bin/bash
Experiment="--- Experiments for RAPIDO 2017 ---"
echo $Experiment
echo $Experiment > log.txt
#number of experiments
total_experiments=5

echo -n "Insert the experiment number (Enter for all experiments) > "
read exp_no
if [ "$exp_no" == "" ]
then
    echo "Running all experiments"
    first_exp=1
    last_exp=$total_experiments
else
    first_exp=$exp_no
    last_exp=$exp_no
    echo "Running experiment "$exp_no
fi


for (( i=$first_exp;i<=$last_exp;i++)); do
    printf "\n" >> log.txt
    echo "--- Experiment "$i" MCR ..." >> log.txt
    date >> log.txt
    ls exp_$i/sdfs >> log.txt
    SECONDS=0
    sudo chrt --rr 99 ./../../bin/adse --config exp_$i/config.cfg --dse.th_prop MCR --output exp_$i/MCR/
    duration=$SECONDS
    echo "$(($duration / 60)) minutes and $(($duration % 60)) seconds elapsed." >> log.txt

    echo "--- Experiment "$i" SSE ..." >> log.txt
    date >> log.txt
    ls exp_$i/sdfs >> log.txt
    SECONDS=0
    sudo chrt --rr 99 ./../../bin/adse --config exp_$i/config.cfg --dse.th_prop SSE --output exp_$i/SSE/ 
    duration=$SECONDS
    echo "$(($duration / 60)) minutes and $(($duration % 60)) seconds elapsed." >> log.txt
    echo "end of experiment "$i >> log.txt
    date >> log.txt

done
exit 

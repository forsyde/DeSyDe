#!/bin/bash
Experiment="--- Experiments for DATE 2018 ---"
echo $Experiment
echo $Experiment > log.txt
#number of experiments
total_experiments=23
total_runs=5

echo -n "Insert the experiment number or first-exp last-exp (Enter for all experiments) > "
read config
IFS=$' ' read first_experiment exp_no <<< "$config"
if [[ "${exp_no:-0}" -gt 0 ]]
then
    first_exp=$first_experiment
    last_exp=$exp_no
    echo "Running experiments "$first_exp" to "$last_exp
else
    if [[ "${first_experiment:-0}" -gt 0 ]]
	then
        first_exp=$first_experiment
        last_exp=$first_experiment
        echo "Running experiment "$first_experiment
    else
	    echo "Running all experiments"
	    first_exp=1
	    last_exp=$total_experiments
    fi
fi


for (( i=$first_exp;i<=$last_exp;i++)); do
    printf "\n" >> log.txt
    for ((r=1;r<=$total_runs;r++)); do
        echo "--- Experiment "$i", run "$r" ..." >> log.txt
        mkdir -p -v exp_$i/run$r/out/
        date >> log.txt
        ls exp_$i/sdfs >> log.txt
        SECONDS=0
        #sudo chrt --rr 99 ./../../bin/adse --config exp_$i/config.cfg --dse.th_prop MCR --output exp_$i/MCR/
        #sudo chrt --rr 99 
        ./../../bin/adse --config exp_$i/config.cfg --dse.th_prop MCR --output exp_$i/run$r/
        duration=$SECONDS
        echo "$(($duration / 60)) minutes and $(($duration % 60)) seconds elapsed." >> log.txt
        echo "end of experiment "$i >> log.txt
        date >> log.txt
    done
done
exit 

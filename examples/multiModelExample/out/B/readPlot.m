#!/usr/bin/octave -qf
 clc
 clear
 close all
 data = csvread('nima_results.csv');
 systemName = 'B';
 %data = csvread([systemName ".csv"]);
 timeLaps = data(1:end,2)./60;
 pow = data(1:end,7);
 pow = 100*pow./max(pow);
 %hist(throughput);
 plot(timeLaps, pow, '-*')
 
 %unique(throughput)
 %return
 %Saving
 filename = [systemName '_real.csv'];
 %--- First write the header
 fid = fopen(filename, 'w');
 fprintf(fid, 'time,power, \n');
 fclose(fid)
 %--- Now append the data
 dlmwrite(filename,[timeLaps pow],'delimiter',',','-append');
 
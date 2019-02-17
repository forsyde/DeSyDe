optimized for power, with restrictions:

rel(*this, noc_mode == 0);
rel(*this, procsUsed == 4);
rel(*this, period[0]!=period[1]);

and 
<constraint app_name="a_sobel" period="15000" latency="8000"></constraint> 
<constraint app_name="b_susan" period="35000" latency="8000"></constraint> 

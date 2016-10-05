#include "flexperiodictasks.hpp"

using namespace std;

  FlexPeriodicTasks::FlexPeriodicTasks(){
    
  }

  FlexPeriodicTasks::FlexPeriodicTasks(vector<FlexPeriodicTask> _tasks):
    tasks(_tasks){

  }

  int FlexPeriodicTasks::getNumberOfTasks(){
    return tasks.size();
  }

  vector<int> FlexPeriodicTasks::getWCEC(int id){
    return tasks[id].wcec;
  }

  int FlexPeriodicTasks::getMinWCEC(int id){
    int wcec=getMaxWCEC(id);
    vector<int> wcecs=tasks[id].wcec;
    for(unsigned i=0; i<wcecs.size(); i++){
      if(wcec>wcecs[i]) wcec=wcecs[i];
    }
    return wcec;
  }

  int FlexPeriodicTasks::getMaxWCEC(int id){
    int wcec=0;
    vector<int> wcecs=tasks[id].wcec;
    for(unsigned i=0; i<wcecs.size(); i++){
      if(wcec<wcecs[i]) wcec=wcecs[i];
    }
    return wcec;
  }

  int FlexPeriodicTasks::getWCEC(int id, int node){
    return tasks[id].wcec[node];
  }

  int FlexPeriodicTasks::getPhase(int id){
    return tasks[id].phase;
  }

  vector<int> FlexPeriodicTasks::getPeriod(int id){
    return tasks[id].period;
  }


  int FlexPeriodicTasks::getNumberOfPeriodCombinations(){
    int combSum = 0;
    vector<int> combs;
    unsigned t = tasks.size();
    unsigned n = (int)pow(2.0, (double)t) - 1;
    
    //cout << n << " task combinations: " << endl;
    for(unsigned i=1; i<=n; i++){
      vector<int> periodValues;
      for(unsigned j=0; j<t; j++){
        if(std::bitset<32>(i)[j]){
          periodValues.push_back(tasks[j].period.size());
        }
      }
      int product=periodValues[0];
      //cout << product;
      for(unsigned j=1; j<periodValues.size(); j++){
        //cout << " * " << periodValues[j]; 
        product *= periodValues[j];
      }
      //cout << " = " << product << endl;
      combs.push_back(product);
      periodValues.clear();
    }
    //for(unsigned j=0; j<combs.size(); j++){
      //cout << std::bitset<8>(j+1) << " " << combs[j] << endl;
      //combSum += combs[j];
    //}
    //cout << "Resulting number of combinations = " << combSum << endl;
    
    return combSum;
  }
  
  //This function determines all possible hyperperiods per scenario,
  //where a scenario is a combination of tasks mapped to the same processor
  // (i.e. index of outer vector = task scenario id, as binary pattern) 
  vector<vector<int>> FlexPeriodicTasks::getHyperperiods(){
    vector<vector<int>> h_periods;
    vector<vector<vector<int>>> occurences;
    h_periods.push_back(vector<int>()); //empty vector for "0"
    
    unsigned t = tasks.size();
    unsigned n = (int)pow(2.0, (double)t) - 1;
    for(unsigned i=1; i<=n; i++){
      cout << " i: " << std::bitset<8>(i) << endl;
      vector<vector<int>> periods;
      for(unsigned j=0; j<t; j++){
        if(std::bitset<32>(i)[j]){
          periods.push_back(tasks[j].period);
          cout << "  " << j << " is in." << endl;
        }
      }
      vector<int> lcm_periods(1, 1);
      for(unsigned i=0; i<periods.size(); i++){
        vector<int> tmp_periods = lcm_periods;
        lcm_periods.clear();
        for(unsigned l=0; l<tmp_periods.size(); l++){
          for(unsigned j=0; j<periods[i].size(); j++){
            int lcm = boost::math::lcm(periods[i][j], tmp_periods[l]);
            lcm_periods.push_back(lcm);
          }
        }
      }
      h_periods.push_back(lcm_periods);
      cout << "  determined " << lcm_periods.size() << " hyperperiods." << endl;
      periods.clear();
    }
    int combSum = 0;
    for(unsigned j=0; j<h_periods.size(); j++){
      //cout << std::bitset<8>(j) << " ";
      //for(unsigned i=0; i<h_periods[j].size(); i++){
        //cout << h_periods[j][i] << " | ";
      //}
      //cout << endl;
      combSum += h_periods[j].size();
    }
    cout << "Resulting number of combinations = " << combSum << endl;
    
    return h_periods;
  }
  
  //This function returns all possible hyperperiods for each task and period
  //outer vector: index = task id
  //inner vector: index = period alternatives
  //most inner vector: possible hyperperiods for the period
  vector<vector<vector<int>>> FlexPeriodicTasks::getHyperperiodsPerTask(){
    vector<vector<vector<int>>> hperiodPerTask;
    for(unsigned i=0; i<tasks.size(); i++){
      hperiodPerTask.push_back(vector<vector<int>>());
      for(unsigned j=0; j<tasks[i].period.size(); j++){
        hperiodPerTask[i].push_back(vector<int>(1,tasks[i].period[j]));
      }  
    }
    vector<vector<int>> h_periods = getHyperperiods();
    for(unsigned i=1; i<h_periods.size(); i++){
      if(std::bitset<32>(i).count() > 1){
        //cout << std::bitset<8>(i) << endl;
        vector<int> taskIds;
        for (size_t t=tasks.size()-1; t>=0; t--){
          if(std::bitset<32>(i)[t]){
            taskIds.push_back(t);
            //cout << t << " ";
          }
        }
        //cout << endl;
        
        for(unsigned j=0; j<h_periods[i].size(); j++){
          unsigned t=0;
          unsigned n=tasks[taskIds[t]].period.size();
          hperiodPerTask[taskIds[t]][j%n].push_back(h_periods[i][j]);
          for(t=1; t<taskIds.size(); t++){
            unsigned p = tasks[taskIds[t]].period.size();
            hperiodPerTask[taskIds[t]][(j/n)%p].push_back(h_periods[i][j]);
            n *= p;
          }
        }
      }
      
    }
    return hperiodPerTask;
  }
  
  
  vector<vector<vector<int>>> FlexPeriodicTasks::getInstancesPerTask(){
    vector<vector<vector<int>>> instPerTask;  
    vector<vector<vector<int>>> hperiodPerTask = getHyperperiodsPerTask();
    
    for(unsigned t=0; t<hperiodPerTask.size(); t++){
      instPerTask.push_back(vector<vector<int>>());
      for(unsigned p=0; p<hperiodPerTask[t].size(); p++){
        int period = hperiodPerTask[t][p][0];
        instPerTask[t].push_back(vector<int>(1,1));
        for(unsigned h=1; h<hperiodPerTask[t][p].size(); h++){
          instPerTask[t][p].push_back(hperiodPerTask[t][p][h]/period);
          
          if(hperiodPerTask[t][p][h]%period != 0){
            cout << "FlexPeriodicTasks::getInstancesPerTask(): an error occured!" << endl;
          } 
        }
      }
    }
    
    return instPerTask;
  }
  
  vector<vector<int>> FlexPeriodicTasks::getMaxInstancesPerPeriod(){
    vector<vector<int>> instPerPeriod;  
    vector<vector<vector<int>>> instPerTask = getInstancesPerTask();  
    
    for(unsigned t=0; t<instPerTask.size(); t++){
      instPerPeriod.push_back(vector<int>());
      for(unsigned p=0; p<instPerTask[t].size(); p++){
        instPerPeriod[t].push_back(1);
        for(unsigned h=1; h<instPerTask[t][p].size(); h++){
          instPerPeriod[t][p] = max(instPerPeriod[t][p], instPerTask[t][p][h]);
        }
      }
    }
    return instPerPeriod;
  }
  
  
  vector<int> FlexPeriodicTasks::getMaxNumberOfInstances(){
    vector<int> maxInstances;
    vector<vector<vector<int>>> instPerTask = getInstancesPerTask();  
    
    for(unsigned t=0; t<instPerTask.size(); t++){
      maxInstances.push_back(1);
      for(unsigned p=0; p<instPerTask[t].size(); p++){
        for(unsigned h=1; h<instPerTask[t][p].size(); h++){
          maxInstances[t] = max(maxInstances[t], instPerTask[t][p][h]);
        }
      }
    }
    
    return maxInstances;
  }


  int FlexPeriodicTasks::getDeadline(int id){
    return tasks[id].deadline;
  }

  vector<int> FlexPeriodicTasks::getMemCons(int id){
    return tasks[id].memCons;
  } 

  int FlexPeriodicTasks::getCodeSize(int id){
    return tasks[id].codeSize;
  }

  bool FlexPeriodicTasks::isPreemtable(int id){
    return tasks[id].preemtable;
  }

  int FlexPeriodicTasks::getMaxCodeSize(){
    int tmp = 0;
    for(unsigned i=0; i<tasks.size(); i++){
      tmp = max(tmp, tasks[i].codeSize);
    }
    return tmp;
  }

  //int FlexPeriodicTasks::getMaxHyperperiod(){
    //int lcm = 1;
    //for(unsigned i=0; i<tasks.size(); i++){
      //int period = tasks[i].period;
      //lcm = boost::math::lcm(lcm, period);
    //}
    //return lcm;
  //}

  int FlexPeriodicTasks::getMinWCEC(){
    int wcec=getMaxWCEC();
    for(unsigned i=0; i<tasks.size(); i++){
      for(unsigned j=0; j<tasks[i].wcec.size(); j++){
        if(wcec>tasks[i].wcec[j]) wcec = tasks[i].wcec[j];
      }
    }
    return wcec;
  }

  int FlexPeriodicTasks::getMaxWCEC(){
    int wcec=0;
    for(unsigned i=0; i<tasks.size(); i++){
      for(unsigned j=0; j<tasks[i].wcec.size(); j++){
        if(wcec<tasks[i].wcec[j]) wcec = tasks[i].wcec[j];
      }
    }
    return wcec;
  }

  //int FlexPeriodicTasks::getMinPeriod(){
    //int period=tasks[0].period;
    //for(unsigned i=1; i<tasks.size(); i++){
      //if(period>tasks[i].period) period = tasks[i].period;
    //}
    //return period;
  //}

  //int FlexPeriodicTasks::getMaxPeriod(){
    //int period=0;
    //for(unsigned i=0; i<tasks.size(); i++){
      //if(period<tasks[i].period) period = tasks[i].period;
    //}
    //return period;
  //}

  //int FlexPeriodicTasks::getMaxNumberOfInstances(){
    //return (getMaxHyperperiod()/getMinPeriod());
  //}


  int FlexPeriodicTasks::areHomogeneous(int nodeI, int nodeJ){
    for(unsigned i=0; i<tasks.size(); i++){
      if(tasks[i].wcec[nodeI]!=tasks[i].wcec[nodeJ]) return false;
    }
    return true;
  }

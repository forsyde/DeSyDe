/**
 * Copyright (c) 2013-2016, Kathrin Rosvall  <krosvall@kth.se>
 *                          Nima Khalilzad   <nkhal@kth.se>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef __DESIGNCONSTRAINTS__
#define __DESIGNCONSTRAINTS__

//using namespace std;

enum SolutionMode { FIRSTSAT, ALLSAT, FIRSTOPT, ALLOPT };

/**
 * This class specifies all supported design constraints.
 * TODO: This could be specified in an XML file.
 */
class DesignConstraints {
public:
  SolutionMode sm;
  Config::OptCriterion oc;
  bool mapSDF;
  float throughputBound;
  int max_period;
  int max_memCons;
  int max_procsUsed;

  DesignConstraints(){
    /*sm = FIRSTOPT;
    oc = THROUGHPUT;
    mapSDF=false;
    max_period = 0;
    //max_period = (1/throughputBound);
    //max_period = 5;
    //throughputBound = 1.0/max_period;
    max_memCons = 0;

    cout << "---------------------------" << endl;
    cout << "Design constraint settings:" << endl;
    switch (sm){
      case FIRSTSAT: cout << "\tFind one satisfying solution." << endl;break;
      case ALLSAT: cout << "\tFind all satisfying solutions." << endl;break;
      case FIRSTOPT: cout << "\tFind one optimal solution." << endl;break;
      case ALLOPT: cout << "\tFind all equally optimal solutions." << endl;break;
      default: cout << "\tSolution setting missing." << endl;break;
    }
    if(sm==FIRSTOPT || sm ==ALLOPT){
      switch (oc){
        case THROUGHPUT: cout << "\tMaximize throughput." << endl;break;
        case MEMORY: cout << "\tMinimize total memory consumption." << endl;break;
        case PROCS_USED: cout << "\tMinimize number of used processors." << endl;break;
        default: cout << "\tOptimization criterion missing." << endl;break;
      }
    }
    if(max_period>0){
      cout << "\tRequired throughput: " << throughputBound << " (1/" << max_period << ")" << endl;
    }else{
      cout << "\tNo throughput bound set." << endl;
    }
    if(max_memCons>0){
      cout << "\tUpper memory bound: " << max_memCons << endl;
    }else{
      cout << "\tNo memory bound set (other than specified in platform)." << endl;
    }
    cout << "---------------------------" << endl;*/
  }
  
  DesignConstraints(SolutionMode _m, Config::OptCriterion _o, int _bound){
    sm = _m;
    oc = _o;
    max_period = _bound;  
  }
  
  void setDesignConstraint(vector<char*> elements, vector<char*> values)
  {
      for(unsigned int i=0;i< elements.size();i++)
        {
            try 
            {            
                if(strcmp(elements[i], "solution") == 0)
                {
                    if(strcmp(values[i], "sat") == 0)
                        sm = FIRSTSAT;
                    if(strcmp(values[i], "opt") == 0)
                        sm = FIRSTOPT;    
                }
                    
                if(strcmp(elements[i], "period") == 0)
                    max_period = atoi(values[i]);
                    
                if(strcmp(elements[i], "criterion") == 0)
                {
                    if(strcmp(values[i], "throughput") == 0)
                        oc = Config::THROUGHPUT;
                }
            }                
            catch(std::exception const & e)
            {
                 cout << "reading design constraints xml file error : " << e.what() << endl;
            }
        }
  }
  
  void setDesignConstraint(SolutionMode _m, Config::OptCriterion _o, int _bound){
    sm = _m;
    oc = _o;
    max_period = _bound;  
  }
    
  SolutionMode getSolutionMode(){ return sm; }

  Config::OptCriterion getOptCriterion(){ return oc; }

  bool isMapSDF(){ return mapSDF;}

  int getPeriodBound(){ return max_period;}
  
  friend std::ostream& operator<< (std::ostream &out, const DesignConstraints &consts)
  {
      string sm;
      switch(consts.sm)
      {
          case(FIRSTSAT):
            sm = "FIRSTSAT";
            break;
        case(ALLSAT):
            sm = "ALLSAT";
            break;
        case(FIRSTOPT):
            sm = "FIRSTOPT";
            break;
        case(ALLOPT):
            sm = "ALLOPT";
            break;                
      }
      out <<     "Solution mode: "     <<    sm                    <<
                " Criterion: "        <<    consts.oc            <<
                " period bound: "    <<    consts.max_period     <<
                endl;
      return out;
  }

};

#endif

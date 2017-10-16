
#include "schedulability.hpp"

using namespace Gecode;
using namespace Int;
using namespace std;


Schedulability::Schedulability( Space& home, 
                                ViewArray<IntView> _wcet,
                                ViewArray<IntView> _proc,
                                ViewArray<IntView> _proc_mode,
                                IntArgs _periods,
                                IntArgs _priorities,
                                IntArgs _deadlines,
                                int _n_actors )
  : Propagator(home), wcet(_wcet), proc(_proc), proc_mode(_proc_mode), 
    periods(_periods), priorities(_priorities), deadlines(_deadlines), 
    n_actors(_n_actors), n_tasks(periods.size()) {

  calls=0;
  total_time=0;
  proc.subscribe(home, *this, Int::PC_INT_VAL);
  home.notice(*this, AP_DISPOSE);
}

size_t Schedulability::dispose(Space& home){
  home.ignore(*this, AP_DISPOSE);
  (void) Propagator::dispose(home);
  return sizeof(*this);
}
 
Propagator* Schedulability::copy(Space& home, bool share){
  return new (home) Schedulability(home, share, *this);
}

PropCost Schedulability::cost(const Space& home, const ModEventDelta& med) const{
  return PropCost::linear(PropCost::HI,wcet.size());
}

void Schedulability::reschedule(Space& home){
  proc.subscribe(home, *this, Int::PC_INT_VAL);
}

Schedulability::Schedulability(Space& home, bool share, Schedulability& p)
  : Propagator(home, share, p),
    wcet(p.wcet),
    proc(p.proc),
    proc_mode(p.proc_mode),
    periods(p.periods),
    priorities(p.priorities),
    deadlines(p.deadlines),
    n_actors(p.n_actors),
    n_tasks(p.n_tasks),
    calls(p.calls) {

  wcet.update(home, share, p.wcet);
  proc.update(home, share, p.proc);
}


ExecStatus Schedulability::propagate(Space& home, const ModEventDelta&){
  
  calls++;
  //cout << "propagate: " << calls << "--------------------" << endl;
  if(FPSchedulable())
    {
      //cout << "proc " << proc  << " \n" ;
      //cout << "wcet " << wcet  << endl;
      //cout << "\t -----SCHEDULABLE-----" << endl;
      if(proc.assigned())
        return home.ES_SUBSUMED(*this);
      else
        return ES_FIX;
    }
  else
    {
      //cout << "proc " << proc << endl;
      //cout << "wcet " << wcet  << endl;
      //cout << "\t -----NOT SCHEDULABLE-----" << endl;
      return ES_FAILED;
    }
  return ES_FIX;
}

int Schedulability::leveliWorkload(int taskid, int t) {
  int procID = proc[taskid].val();
  int W      = wcet[taskid].val();
  for (size_t i=n_actors; i<n_actors+n_tasks; i++)
    {
      /**
       * If task is mapped on my processor and it has higher priority (0 has highest)
       * i is global task id i.e. considering both tasks and actors
       * (i-n_actors) is the local task id: priorities and periodes need local task id
       */  
      if(isOnProc(i, procID) && priorities[taskid-n_actors] > priorities[i-n_actors])
        {
          W += ceil((double)t/periods[i-n_actors]) * wcet[i].val();
        }
    }
  return W;	
}

bool Schedulability::FPSchedulable()
{
  /**
   * goes through all proccessors that mode is assigned
   * first tries utilzation bound test
   * if it fails it tries the time demand test
   */ 
  for (auto k=0;k < proc_mode.size();k++)
    {
      if(proc_mode[k].assigned())
        {
          if(!utilizationBound(k))
            {	
              if(!timedemand(k))
                {
                  //cout << "proc_" << k << " is NOT schedulabale \n" ;
                  return false;
                }
            }
        }
    }
  return true;
}
bool Schedulability::timedemand(int procid)
{
  bool taski;
  for (auto i=n_actors; i<n_actors+n_tasks; i++)
    {
      if(isOnProc(i, procid))
        {
          taski = false;
          for (auto t=1; t<= deadlines[i-n_actors]; t++) //This can be improved by Bini and Buttazzo's approach
            {
              if(leveliWorkload(i, t) <= t)
                {
                  taski = true;
                  break;
                }
            }
          if(!taski)
            {
              //cout << "task_" << i << " is NOT schedulabale \n" ;
              return false;
            }
        }
    }	
  //cout << "proc[" << procid << "] is schedulable based on time demand analysis" << endl;
  return true;
}
bool Schedulability::utilizationBound(int procid)
{
  int n = 0;
  double utils = 0;
  for (size_t i=n_actors; i<n_actors+n_tasks; i++)
    {
      if(isOnProc(i, procid))
        {
          n++;
          utils += ((double) wcet[i].val())/ deadlines[i-n_actors];
        }
    }
  if(n > 0)
    {
      double bound = n*(pow(2, (1.0/n)) - 1);
      //cout << "proc[" << procid << "] bound: "<< bound << " utils: " << utils<< ", no_tasks: " << n  << endl;
      if(utils > bound)
        {
          //cout << "proc[" << procid << "] is NOT schedulable based on utilization bound" << endl;
          return false;
        }
    }
  else
    return true;/// no tasks assigned on this proc --> schedulable
  //cout << "proc[" << procid << "] is schedulable based on utilization bound" << endl;
  return true;
}
bool Schedulability::isOnProc(int taskid, int procid)
{
  if(!proc[taskid].assigned() || proc[taskid].val() != procid) 
    {
      return false;
    }
  return true;
}

void Schedulability( Space& home, 
                     const IntVarArgs& _wcet,
                     const IntVarArgs& _proc,
                     const IntVarArgs& _proc_mode,
                     const IntArgs& _periods,
                     const IntArgs& _priorities,
                     const IntArgs& _deadlines,
                     const int& _n_actors)
{
  if (home.failed()) 
    return;
	
  ViewArray<Int::IntView> tmp_wcet(home, _wcet);
  ViewArray<Int::IntView> tmp_proc(home, _proc);
  ViewArray<Int::IntView> tmp_proc_mode(home, _proc_mode);
  if (_wcet.size() != _proc.size()) {
    throw Gecode::Int::ArgumentSizeMismatch("Schedulability constraint, proc & wcet");
  }
  if (Schedulability::post(home, tmp_wcet, tmp_proc, tmp_proc_mode, _periods, _priorities, _deadlines, _n_actors) != ES_OK) {
    home.fail();
  }
}

#include "pr_taskset.hpp"
#include <algorithm>

using namespace std;

TaskSet::TaskSet() { }

TaskSet::TaskSet(vector<PeriodicTask*> _tasks) 
  : tasks(_tasks),
    Scheduler(FP) { }

TaskSet::TaskSet(XMLdoc& xml)
{
	const char* my_xpathString = "///@name";
	LOG_DEBUG("running xpathString  " + tools::toString(my_xpathString) + " on task set ...");
	for (const auto& name : xml.xpathStrings(my_xpathString)){
		LOG_DEBUG("TaskSet class is parsing the following taskset: " + name + "...");
	}
	load_xml(xml);
}

TaskSet::~TaskSet() {
  for (size_t i=0;i<tasks.size();i++)
    delete tasks[i];
}
void TaskSet::load_xml(XMLdoc& xml)
{
	const char* my_xpathString = "///taskset/periodicTask";
	LOG_DEBUG("running xpathString  " + tools::toString(my_xpathString) + " on task set ...");
	auto xml_tasks = xml.xpathNodes(my_xpathString);
	int task_id = 0;
	for (const auto& task : xml_tasks)
	{
		string task_name = xml.getProp(task, "name");
		string task_type = xml.getProp(task, "type");
		string task_period = xml.getProp(task, "period");
		string task_deadline = xml.getProp(task, "deadline");
		string task_memCons = xml.getProp(task, "memCons");
		string task_number = xml.getProp(task, "number");
		LOG_DEBUG("Reading task with type: " + task_name + "...");		
		for(int i=0; i<atoi(task_number.c_str()); i++)
		{
			PeriodicTask* pr_task = new 
						 PeriodicTask(atoi(task_period.c_str()), atoi(task_memCons.c_str())
									, 0, task_name, task_type, task_id);								
			task_id++;
			tasks.push_back(pr_task);
		}
	}	
}
int TaskSet::getNumberOfTasks() {
  return tasks.size();
}

int TaskSet::getPhase(int id) {
  return tasks[id]->phase;
}

int TaskSet::getPeriod(int id) {
  return tasks[id]->period;
}

vector<int> TaskSet::getHyperperiods() {
  vector<int> h_periods;
  int t = tasks.size();
  int n = (int)pow(2.0, (double)t) - 1;
  for (int i=1; i<=n; i++){
    vector<int> periods;
    for (int j=1; j<=t; j++){
      int mask = (int)pow(2.0, (double)(j-1));
      if((i&mask)==mask){ 
	int x = (int)(log((double)mask)/log(2));
	periods.push_back(tasks[x]->period);
      }
    }
    if(periods.size()==1){
      h_periods.push_back(periods[0]);
    }else{
      int lcm=1;
      for (size_t j=0; j<periods.size(); j++){
	lcm = boost::math::lcm(lcm, periods[j]);
      }
      h_periods.push_back(lcm);
    }
    periods.clear();
  }
  if(h_periods.size() != (unsigned) n) cout << "TaskSet::getHyperperiods() went wrong.\n";
  return h_periods;
}

int TaskSet::getDeadline(int id){
  return tasks[id]->deadline;
}

int TaskSet::getMemCons(int id){
  return tasks[id]->memCons;
} 

int TaskSet::getCodeSize(int id){
  return tasks[id]->codeSize;
}

bool TaskSet::isPreemtable(int id){
  return tasks[id]->preemtable;
}

int TaskSet::getMaxCodeSize(){
  int tmp = 0;
  for (size_t i=0; i<tasks.size(); i++){
    tmp = max(tmp, tasks[i]->codeSize);
  }
  return tmp;
}

int TaskSet::getMaxHyperperiod(){
  int lcm = 1;
  for (size_t i=0; i<tasks.size(); i++){
    int period = tasks[i]->period;
    lcm = boost::math::lcm(lcm, period);
  }
  return lcm;
}

int TaskSet::getMinPeriod(){
  int period=tasks[0]->period;
  for (size_t i=1; i<tasks.size(); i++){
    if(period>tasks[i]->period) period = tasks[i]->period;
  }
  return period;
}

int TaskSet::getMaxPeriod(){
  int period=0;
  for (size_t i=0; i<tasks.size(); i++){
    if(period<tasks[i]->period) period = tasks[i]->period;
  }
  return period;
}

int TaskSet::getMaxNumberOfInstances(){
  return (getMaxHyperperiod()/getMinPeriod());
}

	
bool TaskSet::comparePeriods(PeriodicTask* lhs, PeriodicTask* rhs)  {
  return lhs->period > rhs->period;
}
    
void TaskSet::sortTasksPeriod() {
  for (size_t j=0; j<tasks.size(); j++)
    for (size_t i=0; i<tasks.size()-1; i++)
      {
	if(TaskSet::comparePeriods(tasks[i], tasks[i+1]))
	  {
	    SwapTasks(i, i+1);
	  }
      }		
}
void TaskSet::SwapTasks(int i, int j) {
  using std::swap;
  swap(tasks[i], tasks[j]);
}
std::ostream& operator<< (std::ostream &out, const TaskSet &taskset) {
  for (size_t i=0; i<taskset.tasks.size(); i++)
    {
      out << *taskset.tasks[i] << endl;
    }
  return out;
}
	
void TaskSet::SetRMPriorities() {
  sortTasksPeriod();
  tasks[0]->priority = 0;
  int tmpPriority = 0;
  for (size_t i=1; i<tasks.size(); i++)
    {
      if(tasks[i]->period > tasks[i-1]->period)
	tmpPriority ++; /**< Only increase priority if period is different.*/
      tasks[i]->priority = tmpPriority;
    }		
}

string TaskSet::getTaskType(int id) {
  return tasks[id]->type;
}
void TaskSet::PrintSchedulabilityElapsedTime() {
  std::cout << "Time spent on FP schedulability test: " << float( schedulabilityTime ) /  CLOCKS_PER_SEC  << "\n";
} 

PeriodicTask* TaskSet::getTask(int id) {
  if(id >= (int) tasks.size())
    {
      cout << "pr task index: " << id << " out of bound\n";
    }
  return tasks[id];
}

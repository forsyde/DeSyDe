#include "pr_task.hpp"

using namespace std;

  PeriodicTask::PeriodicTask(int _phase, int _period, int _deadline, int _mem, int _cS, int _priority, string _name, string _type, int _id):
    phase(_phase),
    period(_period),
    deadline(_deadline),
    codeSize(_cS),
    priority(_priority),
    name(_name),
    type(_type),
    id(_id),
    memCons(_mem),    
    preemtable(true){};

  PeriodicTask::PeriodicTask(int _period, int _mem, int _cS, string _name, string _type, int _id):
    phase(0),
    period(_period),
    deadline(_period),
    codeSize(_cS),
    priority(0),
    name(_name),
    type(_type),
    id(_id),
    memCons(_mem),
    preemtable(true){};
	
	PeriodicTask::PeriodicTask(vector<char*> elements, vector<char*> values, int number):
    phase(0),
    period(0),
    deadline(0),
    codeSize(0),
    priority(0),
    name(""),
    type(""),
    id(0),
    preemtable(true)
	{
		for (size_t i=0;i< elements.size();i++)
		{
			try 
			{			
				if(strcmp(elements[i], "phase") == 0)
					phase = atoi(values[i]);
					
				if(strcmp(elements[i], "period") == 0)
					period = atoi(values[i]);
					
				if(strcmp(elements[i], "deadline") == 0)
					deadline = atoi(values[i]);
					
				if(strcmp(elements[i], "priority") == 0)
					priority = atoi(values[i]);
				
				if(strcmp(elements[i], "name") == 0)
					name = string(values[i]);
					
				if(strcmp(elements[i], "type") == 0)
				{
					type = string(values[i]);
					name = type + to_string(number);
				}
						
				if(strcmp(elements[i], "id") == 0)
					id = atoi(values[i]);
				
				if(strcmp(elements[i], "memCons") == 0)
					memCons = atoi(values[i]);
					
				if(strcmp(elements[i], "preemtable") == 0)
					preemtable = atoi(values[i]);
			}				
			catch(std::exception const & e)
			{
				 cout << "reading taskset xml file error : " << e.what() << endl;
			}
		}
	};
	
	std::ostream& operator<< (std::ostream &out, const PeriodicTask &task)
	{
		out << "pr task:" 		<< task.name 		<< "[id=" 	<< task.id
			<< ", type=" 		<< task.type 
			<< ", priority=" 	<< task.priority
			<< "](" 			<< task.period 		<< ", " 	<< task.deadline << ")";
	 
		return out;
	}

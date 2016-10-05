#include <math.h>
#include <iostream>
#include <vector>
#include <boost/math/common_factor.hpp>
#include <gecode/gist.hh>

using namespace std;

class ModelExec{
public:
	template <class T>
	void ExecModel (T a, int mode)
	{
		Gist::Print<T> p("Print solution");
		Gist::Options o;
		o.inspect.click(&p);
		Gist::bab(model,o);
		delete model;
		break;
		return;
	}
	
};

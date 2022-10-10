#include "General.h"

#include <iostream>
using std::cerr;


Profiler::Profiler(std::string msg) : message(move(msg))
{
	start = clock();
}
Profiler::~Profiler()
{
	cerr << "\nEnd " << message << " duration: " << (clock() - start) << " msec" << std::endl;
}

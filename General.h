#pragma once
#include <time.h>
#include <string>

typedef int sort_type;

constexpr size_t bytes_in_KB = 1024;
constexpr size_t bytes_in_MB = 1024 * 1024;
constexpr size_t pref_size = bytes_in_MB * 1;

class Profiler
{
private:
	clock_t start;
	std::string message;

public:
	explicit Profiler(std::string msg);
	~Profiler();
};
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <random>

#include "Algs.h"

#include <Windows.h>

using std::cout;
using std::cin;

constexpr char fileAname[] = "A.bin";
constexpr char fileBname[] = "B.bin";
constexpr char fileCname[] = "C.bin";

constexpr size_t min_entities_for_sort = 4;


void Set(size_t &file_size ,size_t &mem_avail)
{
	size_t mult = 0;
	do
	{
		char ch;
		std::cout << "Choose Units: \n"
			"\t1 - bytes\n"
			"\t2 - KBs\n"
			"\t3 - MBs\n";
		cin >> ch;
		switch (ch)
		{
		case '2': mult = bytes_in_KB; break;
		case '3': mult = bytes_in_MB; break;
		case '1': mult = 1;

		}
	} while (!mult);
	
	size_t real_mem_avail, real_file_size;
	cout << "Enter file size: "; cin >> real_file_size;
	cout << "Eneter available memory: "; cin >> real_mem_avail;

	mem_avail = (real_mem_avail * mult / min_entities_for_sort / sizeof(sort_type)) * sizeof(sort_type) * min_entities_for_sort;
	file_size = (real_file_size * mult / sizeof(sort_type) * sizeof(sort_type));
	cout << "fs: " << file_size << '\n';
	cout << "ma: " << mem_avail << '\n';

	if (mem_avail < sizeof(sort_type) * 8)
	{
		std::cerr << "UB: Too few memory!";
		exit(-1);
	}
	if (mem_avail >= file_size)
	{
		std::cerr << "Use internal sort!";
		exit(-1);
	}
}


int main()
{

	size_t mem_avail, file_size;
	Set(file_size, mem_avail);

	char* raw_data = new char[mem_avail];


	do
	{
		char ch;
		cout << "Generate | Sort | Check |Exit\n";
		cin >> ch;
		switch (ch)
		{
		case 'G':
		case 'g': 
		{
			Profiler g("Generation");
			Generate(fileAname, file_size, (sort_type*)raw_data, mem_avail);
		}
		break;
		case 'S':
		case 's':
		{
			size_t peffered_mem_avail = mem_avail > pref_size ? pref_size : mem_avail;
			{
				SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);
				Profiler sort("Sorting " + std::to_string(file_size) + " bytes with " + std::to_string(mem_avail) + " bytes memory available");
				
				{
					Profiler sort("Presort");
					PreSort(fileAname, (sort_type*)raw_data, mem_avail);
				}
				MappedFile A(fileAname, (sort_type*)raw_data, peffered_mem_avail / 2);
				MappedFile B(fileBname, (sort_type*)(raw_data + peffered_mem_avail / 2), peffered_mem_avail / 4);
				MappedFile C(fileCname, (sort_type*)(raw_data + 3 * peffered_mem_avail / 4), peffered_mem_avail / 4);

				while (!Devide(A, B, C))
				{
					Merge(A, B, C);
				}
				A.Close(); B.Close(); C.Close();
				SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
			}
			remove(fileBname);
			remove(fileCname);
			std::cout << "is sorted: " << std::boolalpha << IsSorted(fileAname, (sort_type*)raw_data, mem_avail) << '\n';

		}
		break;
		case 'E':
		case 'e': return 0; break;
		case 'C':
		case 'c': std::cout << "is sorted: " << std::boolalpha << IsSorted(fileAname, (sort_type*)raw_data, mem_avail) << '\n'; break;

		default:
			break;
		}

	} while (true);

	return 0;
}
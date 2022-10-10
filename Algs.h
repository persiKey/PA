#pragma once
#include "MappedFile.h"
using std::string;

void Generate(string filename, size_t file_size, int* raw_memory, size_t mem_size);


bool Devide(MappedFile& A, MappedFile& B, MappedFile& C);

void Merge(MappedFile& A, MappedFile& B, MappedFile& C);


bool IsSorted(string filename, sort_type* raw_memory, size_t mem_size);

void PreSort(string filename, sort_type* raw_memory, size_t mem_size);

void DropFilePatially(FILE* file, char* data, size_t size);
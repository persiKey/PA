#include "Algs.h"
#include <stdlib.h>
#include <time.h>
#include <random>
#include <execution>

void Generate(string filename, size_t file_size, int* raw_memory, size_t mem_size)
{
	FILE* file;
	fopen_s(&file, filename.c_str(), "wb");
	size_t num_of_el, been_write = 0, fit = file_size / sizeof(int);
	std::mt19937 Generator;

	int MN = time(0);

	Generator.seed(MN);
	int fit_size = fit * sizeof(int);
	do
	{
		num_of_el = std::min(mem_size / sizeof(int), fit - been_write);
		been_write += num_of_el;
		for (size_t i = 0; i < num_of_el; ++i)
		{
			raw_memory[i] = Generator() % 30;
		}
		fwrite(raw_memory, num_of_el * sizeof(int), 1, file);
	} while (fit != been_write);
	fclose(file);
}

bool Devide(MappedFile& A, MappedFile& B, MappedFile& C)
{
	A.ToBegin();
	B.Trunc();
	C.Trunc();

	bool flag = true;

	sort_type buf = 0, old = 0;


	old = A.Read();
	while (true)
	{
		MappedFile& Active = flag ? B : C;
		flag = !flag;

		do
		{
			Active.Write(old);

			buf = A.Read();
			if (A.Eof())
				break;
			if (old <= buf)
			{
				old = buf;
			}
			else
			{
				old = buf;
				break;
			}

		} while (true);
		if (A.Eof())
		{
			B.Flush();
			C.Flush();
			return	C.Empty();
		}
	}
}


void Merge(MappedFile& A, MappedFile& B, MappedFile& C)
{
	A.ToBegin(false);
	B.ToBegin();
	C.ToBegin();


	sort_type old_B, old_C;
	old_B = B.Read();
	old_C = C.Read();
	sort_type buf_B, buf_C;

	while (true)
	{
		if (old_B <= old_C)
		{
			A.Write(old_B);

			buf_B = B.Read();
			if (B.Eof())
			{
				old_B = buf_B;
				goto appendc;
			}
			if (buf_B < old_B)
			{
				do
				{
					A.Write(old_C);
					buf_C = C.Read();

					if (C.Eof())
					{
						old_B = buf_B;
						goto appendb;
						break;
					}
					if (old_C <= buf_C)
					{
						old_C = buf_C;
					}
					else
					{
						old_C = buf_C;
						break;
					}
				} while (true);
			}
			old_B = buf_B;
		}
		else
		{
			A.Write(old_C);

			buf_C = C.Read();
			if (C.Eof())
			{
				old_C = buf_C;
				goto appendb;
			}
			if (buf_C < old_C)
			{
				do
				{
					A.Write(old_B);
					buf_B = B.Read();
					if (B.Eof())
					{
						old_C = buf_C;
						goto appendc;
						break;
					}
					if (old_B <= buf_B)
					{
						old_B = buf_B;
					}
					else
					{
						old_B = buf_B;
						break;
					}
				} while (true);
			}
			old_C = buf_C;
		}
	}
	if (B.Eof())
	{
	appendc:
		A.Write(old_C);
		if (C.Eof())
		{
			A.Flush();
		}
		else
		{
			A += C;
		}
		return;
	}
	if (C.Eof())
	{
	appendb:
		A.Write(old_B);

		if (B.Eof())
		{
			A.Flush();
		}
		else
		{
			A += B;
		}
		return;
	}

}


bool IsSorted(string filename, sort_type* raw_memory, size_t mem_size)
{
	FILE* file;
	fopen_s(&file,filename.c_str(), "rb");
	fseek(file, 0, SEEK_SET);

	sort_type left;
	sort_type right;

	size_t beg = 0;
	size_t end = fread(raw_memory, 1, mem_size, file);
	if (!std::is_sorted(raw_memory, raw_memory + end / sizeof(sort_type)))
	{
		fclose(file);
		return false;
	}
	while (true)
	{
		left = raw_memory[end / sizeof(sort_type) - 1];
		size_t beg = 0;
		size_t end = fread(raw_memory, 1, mem_size, file);
		right = raw_memory[0];
		if (feof(file))
			break;
		if (!std::is_sorted(raw_memory, raw_memory + end / sizeof(sort_type)))
		{
			fclose(file);
			return false;
		}
		if(left > right)
		{
			fclose(file);
			return false;
		}
	}
	fclose(file);



	return true;
}

void PreSort(string filename, sort_type* raw_memory, size_t mem_size)
{
	FILE* file;
	fopen_s(&file, filename.c_str(), "rb+");
	fseek(file, 0, SEEK_SET);

	size_t end, old_pos;
	do
	{
		old_pos = ftell(file);
		end = fread(raw_memory, 1, mem_size, file);
		std::sort(std::execution::par_unseq,
			raw_memory, raw_memory + end / sizeof(sort_type));
		fseek(file, old_pos, SEEK_SET);
		//fwrite(raw_memory, 1, mem_size, file);
		DropFilePatially(file,(char*) raw_memory, mem_size);
		fflush(file);
	} while (feof(file));

	fseek(file, mem_size / 2, SEEK_SET);

	do
	{
		old_pos = ftell(file);
		end = fread(raw_memory, 1, mem_size, file);
		std::sort(std::execution::par_unseq,
			raw_memory, raw_memory + end / sizeof(sort_type));
		fseek(file, old_pos, SEEK_SET);

		DropFilePatially(file, (char*)raw_memory, mem_size);
		fflush(file);
	} while (feof(file));

	fclose(file);
}


void DropFilePatially(FILE* file, char* data, size_t size)
{
	size_t num = size / pref_size;
	for (size_t i = 0; i < num; i++)
	{
		fwrite(data, pref_size, 1, file);
		fflush(file);
		data += pref_size;
	}
	fwrite(data + pref_size * num, size - pref_size * num, 1, file);

}
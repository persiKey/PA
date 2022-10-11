#include "Algs.h"
#include <stdlib.h>
#include <time.h>
#include <random>
#include <execution>

#include <fstream>

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
			raw_memory[i] = Generator();
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
	std::fstream file(filename, std::ios_base::binary | std::ios_base::in | std::ios_base::out);

	size_t end =0, old_pos = 0 ,
		been_read = 0;
	do
	{
		old_pos = file.tellg();
		//printf("presort %zu\n" , old_pos);
		file.read((char*)raw_memory, mem_size);
		end = file.gcount();
		std::sort(std::execution::par_unseq,
			raw_memory, raw_memory + end / sizeof(sort_type));
		file.seekp(old_pos, std::ios_base::beg);
		old_pos += end;
		DropFilePatially(file, (char*)raw_memory, end);
		file.flush();
	} while (!file.eof());

	file.clear();

	file.seekp(mem_size / 2, std::ios_base::beg);

	do
	{
		old_pos = file.tellg();
		//printf("presort / 2 %zu\n", old_pos);
		file.read((char*)raw_memory, mem_size);
		end = file.gcount();
		std::sort(std::execution::par_unseq,
			raw_memory, raw_memory + end / sizeof(sort_type));
		file.seekp(old_pos, std::ios_base::beg);
		old_pos += end;
		DropFilePatially(file, (char*)raw_memory, end);
		file.flush();
	} while (!file.eof());

	file.clear();
	file.close();
}


void DropFilePatially(std::fstream& file, char* data, size_t size)
{
	size_t num = size / pref_size;
	for (size_t i = 0; i < num; i++)
	{
		file.write(data, pref_size);
		file.flush();
		data += pref_size;
	}
	file.write(data + pref_size * num, size - pref_size * num);
}
#pragma once
#include <string>
#include "General.h"

using std::string;

class MappedFile
{
private:
	string _filename;
	FILE* raw_file;

	sort_type* _data;
	size_t pos = 0;
	size_t chunk_size;
	size_t filesize;
	size_t been_read = 0;
	size_t to_read;
	size_t to_write;

	void read_chunk();
	void write_chunk();

	MappedFile(const MappedFile&) = delete;
	const MappedFile& operator=(const MappedFile& value_) = delete;
public:
	MappedFile(string filename, sort_type* data, size_t size);
	~MappedFile();
	sort_type Read();
	void Write(sort_type);
	bool Eof(); //sus
	void ToBegin(bool read = true);
	void Trunc();
	bool Empty();
	void Flush();
	void Close();
	void operator+=(MappedFile&);
};

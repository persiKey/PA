#include "MappedFile.h"

#include <filesystem>
#include <iostream>
#include <io.h>
#include <assert.h>
using std::ofstream;
using std::ios_base;


void MappedFile::read_chunk()
{
	assert(filesize != 0);
	to_read = std::min(chunk_size, filesize - been_read);
	fread(_data, to_read, 1, raw_file);
	been_read += to_read;

}

void MappedFile::write_chunk()
{
	to_write = std::min(chunk_size, pos * sizeof(sort_type));
	fwrite(_data, to_write, 1, raw_file);
	pos = 0;
}

MappedFile::MappedFile(string filename, sort_type* data, size_t size) : _filename(filename), _data(data), chunk_size(size)
{
	FILE* dummy;
	fopen_s(&dummy, _filename.c_str(), "ab");
	fclose(dummy);

	fopen_s(&raw_file, _filename.c_str(), "rb+");
	

	filesize = std::filesystem::file_size(_filename);

	if (filesize) read_chunk();
}

MappedFile::~MappedFile()
{
	Close();
}

sort_type MappedFile::Read()
{
	if (pos * sizeof(sort_type) >= chunk_size)
	{
		pos = 0;
		read_chunk();
	}

	return _data[pos++];
}



void MappedFile::Write(sort_type buf)
{
	if (pos * sizeof(sort_type) >= chunk_size)
	{
		write_chunk();
	}
	_data[pos++] = buf;
}

bool MappedFile::Eof()
{
	return (pos * sizeof(sort_type) > to_read) || !to_read;
}

void MappedFile::ToBegin(bool read)
{
	rewind(raw_file);
	pos = been_read = 0;
	if (read) read_chunk();
	clearerr(raw_file);
}

void MappedFile::Trunc()
{
	_chsize(_fileno(raw_file), 0);

	filesize = 0;

	ToBegin(false);
}

bool MappedFile::Empty()
{

	return (std::filesystem::file_size(_filename) == 0);
}

void MappedFile::Flush()
{
	if (!been_read && pos)
	{
		write_chunk();
	}
	fflush(this->raw_file);
	filesize = std::filesystem::file_size(_filename);;
	clearerr(raw_file);
	pos = 0;

}

void MappedFile::Close()
{
	fclose(raw_file);
}

void MappedFile::operator+=(MappedFile& Another)
{
	fwrite(this->_data, pos * sizeof(sort_type), 1, this->raw_file);
	fwrite(Another._data + Another.pos, Another.to_read - Another.pos * sizeof(sort_type), 1, this->raw_file);

	if (Another.pos != 0)
	{
		while (Another.to_read == Another.chunk_size)
		{
			Another.read_chunk();
			fwrite(Another._data, Another.to_read, 1, this->raw_file);
		}
	}
	clearerr(this->raw_file);
	fflush(this->raw_file);
	this->filesize = std::filesystem::file_size(this->_filename);
	this->pos = 0;
}

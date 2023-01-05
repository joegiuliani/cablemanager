#include "cmsutil.h"
#include <string>
#include <iostream>
#include <istream>

std::string FILE_EXTENSION = ".cms";

CMSReader::CMSReader(std::string name)
{
	file.open(name + FILE_EXTENSION);
	level_tracker.push(0);
}
CMSReader::~CMSReader()
{
	file.close();
}

bool CMSReader::find(char c)
{
	char rc;
	while (file.get(rc))
	{
		if (c == rc) return true;
	}

	return false;
}

bool CMSReader::has_next_key()
{
	if (find('['))
	{
		file.unget();
		return true;
	}

	return false;
}

std::string CMSReader::open_key()
{               
	if (!find('[')) throw std::exception("No key found");

	std::string ret = "";
	char c;
	while (file.get(c))
	{
		if (c == ']') break;
		ret += c;
	}

	level_tracker.push(0);

	if (!find('{')) throw std::exception("No body found for key");

	return ret;
}

void CMSReader::close_key()
{
	if (!find('}')) throw std::exception("No close bracket found for current key");
	level_tracker.pop();
}

bool CMSReader::has_next_value()
{
	if (find('\"'))
	{
		file.unget();
		return true;
	}

	return false;
}

std::string CMSReader::next_value()
{
	if (!find('\"')) throw std::exception("No value found");

	std::string str = "";
	char c;
	while (file.get(c))
	{
		if (c == '\"') break;

		str += c;
	}

	return str;
} 

CMSWriter::CMSWriter(std::string scene_name)
{
	file.open(scene_name + FILE_EXTENSION, std::ios::trunc);
	depth_tracker.push(0);
}

CMSWriter::~CMSWriter()
{
	file.close();
}

void CMSWriter::add_data(std::string data)
{
	// Add comma if needed
	if (depth_tracker.size() && depth_tracker.top() > 0)
		file << ',';

	file << data;

	depth_tracker.top()++;
}

void CMSWriter::open_key(std::string name)
{
	add_data('[' + name + "]");

	file << '{';
	depth_tracker.push(0);
};

void CMSWriter::close_key()
{
	file << "}";
	depth_tracker.pop();
};

void CMSWriter::add_value(std::string val)
{
	add_data('\"' + val + '\"');
};

void CMSWriter::add_vec(const glm::vec2& vec)
{
	add_value(std::to_string(vec.x));
	add_value(std::to_string(vec.y));
};
#include "cmsutil.h"
#include <string>
#include <iostream>

using namespace CMSUtil;

std::string FILE_EXTENSION = ".cms";

int CMSUtil::count(const std::string& str, char c)
{
	int ct = 0;
	size_t index = str.find(c, 0);
	while (index != std::string::npos)
	{
		ct++;
		index = str.find(c, index);
	}
	return ct;
}

std::string CMSUtil::remove_white_space(const std::string& str)
{
	std::string ret = "";

	bool in_quotes = false;
	for (const char& c : str)
	{
		if (c == '\"')
		{
			in_quotes = !in_quotes;
			continue;
		}

		if (in_quotes) ret += c;

		else if (c == '\n' || c == ' ' || c == '\t')
		{
			continue;
		}

		else ret += c;
	}

	return ret;
}

std::string CMSUtil::read_block(std::ifstream& file, char open_delim, char close_delim)
{
	std::string sum = "";

	int num_open = 0;

	char c = file.get();
	while (!file.eof() && c != open_delim)
	{
		if (c == close_delim)
		{
			std::cout << "Invalid arguments. Found first close delim before first open delim\n";
			return "";
		}

		c = file.get();
	}

	if (file.eof())
	{
		std::cout << "No open delim found";
		return "";
	}

	num_open = 1;
	sum += open_delim;
	int num_close = 0;

	while (!file.eof() && num_open > num_close)
	{
		char c = file.get();
		sum += c;
		// TODO check if line contains '\n'

		if (c == open_delim) num_open++;
		else if (c == close_delim) num_close++;
	}

	if (num_open > num_close)
	{
		std::cout << "NOt enough close delims found in file\n";
		return "";
	}

	return remove_white_space(sum.substr(1, sum.find_last_of(close_delim) - 1));
}

std::string CMSUtil::read_block(const std::string& str, char open_delim, char close_delim)
{
	int cursor = 0;
	int num_open = 0;

	while (cursor < str.length() && str[cursor] != open_delim)
	{
		if (str[cursor] == close_delim)
		{
			std::cout << "Invalid arguments. Found first close delim before first open delim\n";
			return "";
		}
		cursor++;
	}

	if (cursor == str.length())
	{
		std::cout << "No open delim found";
		return "";
	}

	num_open = 1;
	size_t first_delim_index = cursor;
	cursor++;

	int num_close = 0;
	while (cursor < str.length())
	{
		char c = str[cursor];

		if (c == open_delim) num_open++;
		else if (c == close_delim)
		{
			num_close++;
			if (num_close == num_open)
			{
				break;
			}
		}

		cursor++;
	}

	if (cursor == str.length())
	{
		std::cout << "Invalid arguments. Not enough close delims found\n";
		return "";
	}

	std::string contents = str.substr(first_delim_index + 1, cursor - first_delim_index - 1);
	std::string contents_no_white_space = remove_white_space(contents);

	return contents_no_white_space;
}

size_t CMSUtil::find_at_scope(const std::string& str, char c, size_t offset)
{
	int num_open = 0;
	int num_close = 0;
	for (size_t k = offset; k < str.length(); k++)
	{
		if (str[k] == '{') num_open++;
		if (str[k] == '}') num_close++;

		if (num_open == num_close)
		{
			if (str[k] == c)
				return k;
		}
	}

	return std::string::npos;
}

std::vector<float> CMSUtil::read_floats(const std::string& block)
{
	int cursor = 0;
	size_t comma_pos = block.find(',');
	std::vector<float> floats_read;

	while (comma_pos != std::string::npos && cursor < block.length())
	{
		floats_read.push_back(std::stof(block.substr(cursor, comma_pos - cursor)));
		cursor = comma_pos + 1;
		comma_pos = block.find(',', cursor);
	}

	floats_read.push_back(std::stof(block.substr(cursor)));

	return floats_read;
}

Writer::Writer(std::string scene_name)
{
	file.open(scene_name + FILE_EXTENSION, std::ios::trunc);
	depth_tracker.push(0);
}

Writer::~Writer()
{
	file.close();
}

void Writer::add_comma_if_needed()
{
	if (depth_tracker.size() && depth_tracker.top() > 0)
		file << ',';
};

void Writer::open_key(std::string name)
{
	add_comma_if_needed();

	name = '[' + name + "] {";
	file << name;

	depth_tracker.top()++;

	depth_tracker.push(0);
};

void Writer::close_key()
{
	file << "}";
	depth_tracker.pop();
};

void Writer::add_value(std::string val)
{
	add_comma_if_needed();

	file << val;
	depth_tracker.top()++;
};

void Writer::add_vec(const glm::vec2& vec)
{
	add_value(std::to_string(vec.x));
	add_value(std::to_string(vec.y));
};
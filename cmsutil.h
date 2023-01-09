#pragma once
#include <string>
#include <stack>
#include <fstream>
#include <vector>
#include <glm/vec2.hpp>


class CMSWriter
{
public:

	CMSWriter(std::string scene_name);

	~CMSWriter();

	void open_key(std::string name);

	void close_key();

	void add_value(std::string val);

	void add_vec(const glm::vec2& vec);

private:
	std::ofstream file;
	std::stack<int> depth_tracker;
	void add_data(std::string data);
};
class CMSReader
{

public:
	CMSReader(std::string name);
	~CMSReader();
	bool has_next_key();
	std::string open_key();
	void close_key();
	bool has_next_value();
	std::string next_value();
private:
	std::ifstream file;
	std::stack<int> level_tracker;
	bool find(char c);
};
#pragma once
#include <string>
#include <stack>
#include <fstream>
#include <vector>
#include <glm/vec2.hpp>

namespace CMSUtil
{
	int count(const std::string& str, char c);
	std::string remove_white_space(const std::string& str);

	std::string read_block(std::ifstream& file, char open_delim, char close_delim);

	std::string read_block(const std::string& str, char open_delim, char close_delim);

	size_t find_at_scope(const std::string& str, char c, size_t offset = 0);

	std::vector<float> read_floats(const std::string& block);

	class Writer
	{
	public:

		Writer(std::string scene_name);

		~Writer();

		void add_comma_if_needed();

		void open_key(std::string name);

		void close_key();

		void add_value(std::string val);

		void add_vec(const glm::vec2& vec);

	private:
		std::ofstream file;
		std::stack<int> depth_tracker;
	};
}

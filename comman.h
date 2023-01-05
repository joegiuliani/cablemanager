#pragma once

#include <memory>
#include <stack>

class Command
{
public:
	virtual void execute() = 0;
	virtual void reverse() = 0;
};

class CommandManager
{
public:
	template<typename T>
	void add_command(T command)
	{
		clear_stack(available_redos);

		available_undos.push(std::make_shared<T>(command));
		available_undos.top()->execute();
	}


	void undo();

	void redo();
private:
	typedef std::stack<std::shared_ptr<Command>> command_stack;
	command_stack available_undos;
	command_stack available_redos;
	void clear_stack(command_stack& s);

};

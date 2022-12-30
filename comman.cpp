#include "comman.h"
#include <stack>
#include <memory>

typedef std::stack<std::shared_ptr<Command>> command_stack;
command_stack available_undos;
command_stack available_redos;

void clear_stack(command_stack& s)
{
	while (!s.empty())
		s.pop();
}

template<typename T>
void CommandManager::add_command(T command)
{
	clear_stack(available_redos);

	available_undos.push(std::make_shared<T>(command));
	available_undos.top()->execute();
}

void CommandManager::undo()
{
	if (available_undos.size())
	{
		available_undos.top()->reverse();
		available_redos.push(available_undos.top());
		available_undos.pop();
	}
}

void CommandManager::redo()
{
	if (available_redos.size())
	{
		available_redos.top()->execute();
		available_undos.push(available_redos.top());
		available_redos.pop();
	}
}



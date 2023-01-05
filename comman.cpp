#include "comman.h"

void CommandManager::clear_stack(command_stack& s)
{
	while (!s.empty())
		s.pop();
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



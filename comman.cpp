#include "comman.h"

#include "qgl.h"
#include "draw.h"

typedef std::set<int> KeySet;

const KeySet UNDO = { draw::KEY_LEFT_CONTROL, draw::KEY_Z };
const KeySet REDO = { draw::KEY_LEFT_CONTROL, draw::KEY_LEFT_SHIFT, draw::KEY_Z };

void CommandManager::process_key_events()
{
	if (qgl::Keyboard::matches(UNDO))
		undo();
	else if (qgl::Keyboard::matches(REDO))
		redo();
}

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



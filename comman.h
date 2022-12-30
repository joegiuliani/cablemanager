#pragma once
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
	void add_command(T command);

	void undo();

	void redo();
};

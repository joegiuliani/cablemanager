#pragma once

#include <memory>
#include <stack>

namespace cm
{
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
		static void add_command(T command)
		{
			clear_stack(available_redos);

			available_undos.push(std::make_shared<T>(command));
			available_undos.top()->execute();
		}

		static void undo();
		static void redo();

		static void process_key_events();

	private:
		typedef std::stack<std::shared_ptr<Command>> command_stack;
		static inline command_stack available_undos;
		static inline command_stack available_redos;
		static void clear_stack(command_stack& s);
	};

}

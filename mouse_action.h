#pragma once
#include "types.h"
#include "node.h"
#include "qgl.h"

namespace cm
{
	namespace MouseAction
	{
		class PressNode
		{
		public:
			static void init();
			static void press_callback();
		};

		class DragSelect
		{
		public:
			static inline vec start;
			static inline vec end;
			static void begin();
			static void move_callback();
			static void release_callback();
			static void init();
		private:
			static inline qgl::Shape box;
		};

		class DragNode
		{
		public:
			static inline vec start;
			static void begin();
			static void move_callback();
			static void release_callback();
		};

		class Zoom
		{
		public:
			static void scroll_callback();
		};
		class Pan
		{
		public:
			static void begin();
			static void move_callback();
			static void release_callback();
		};
	}

}

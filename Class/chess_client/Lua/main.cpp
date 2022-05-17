#include <iostream>

#pragma comment(lib, "lua54.lib")
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

int main()
{
	const char* command = "print \"Hello, world!\"";

	lua_State* my_lua = luaL_newstate();
	luaL_openlibs(my_lua);
	luaL_loadfile(my_lua, "monster.lua");
	//luaL_loadbuffer(my_lua, command, strlen(command), "line");

	if (int error = lua_pcall(my_lua, 0, 0, 0))
	{
		std::cout << "Error! " << lua_tostring(my_lua, -1) << ".\n";
		lua_pop(my_lua, 1);
		return 1;
	}

	int x_pos, y_pos;
	lua_getglobal(my_lua, "pos_x");
	lua_getglobal(my_lua, "pos_y");
	
	x_pos = lua_tonumber(my_lua, -2);
	y_pos = lua_tonumber(my_lua, -1);
	std::cout << "x: " << x_pos << ", y: " << y_pos << "\n";

	lua_close(my_lua);

	return 0;
}

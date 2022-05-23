#include <iostream>

#pragma comment(lib, "lua54.lib")
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

int test_addnum(lua_State* state)
{
	int lhs = int(lua_tonumber(state, -2));
	int rhs = int(lua_tonumber(state, -1));
	int result = lhs + rhs;
	lua_pop(state, 3);
	lua_pushnumber(state, result);
	return 1;
}

int main()
{
	const char* command = "print \"Hello, world!\"";

	lua_State* my_lua = luaL_newstate();
	luaL_openlibs(my_lua);
	luaL_loadfile(my_lua, "monster.lua");
	lua_pcall(my_lua, 0, 0, 0);
	//luaL_loadbuffer(my_lua, command, strlen(command), "line");
	
	//
	lua_getglobal(my_lua, "plustwo");
	lua_pushnumber(my_lua, 100);
	if (int error = lua_pcall(my_lua, 1, 1, 0))
	{
		std::cout << "Error! " << lua_tostring(my_lua, -1) << ".\n";
		lua_pop(my_lua, 1);
		return 1;
	}
	int plustwo_result = int(lua_tonumber(my_lua, -1));
	std::cout << "plustwo: " << plustwo_result << "\n.";
	lua_pop(my_lua, 1);

	//
	lua_register(my_lua, "test_addnum", test_addnum);
	lua_getglobal(my_lua, "test_wrapper_addnum");
	lua_pushnumber(my_lua, 300);
	lua_pushnumber(my_lua, 400);
	lua_pcall(my_lua, 2, 1, 0);
	int ccall_result = int(lua_tonumber(my_lua, -1));
	std::cout << "c call test: " << ccall_result << "\n.";
	lua_pop(my_lua, 1);

	int x_pos, y_pos;
	lua_getglobal(my_lua, "pos_x");
	lua_getglobal(my_lua, "pos_y");

	x_pos = lua_tonumber(my_lua, -2);
	y_pos = lua_tonumber(my_lua, -1);
	std::cout << "x: " << x_pos << ", y: " << y_pos << "\n";

	lua_close(my_lua);

	return 0;
}

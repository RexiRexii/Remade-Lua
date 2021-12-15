#include "headers.hpp"
#include <Windows.h>

void r_lua_pushnil(std::uintptr_t rL)
{
	r_setnilvalue(*reinterpret_cast<r_TValue**>(rL + offsets::top));
	r_incr_top(rL);
}

void r_lua_pushthread(std::uintptr_t rL)
{
	r_setthvalue(*reinterpret_cast<r_TValue**>(rL + offsets::top), rL);
	r_incr_top(rL);
}

void r_lua_pushboolean(std::uintptr_t rL, bool b)
{
	r_setbvalue(*reinterpret_cast<r_TValue**>(rL + offsets::top), b);
	r_incr_top(rL);
}

void r_lua_pushnumber(std::uintptr_t rL, std::double_t num)
{
	r_setnvalue(*reinterpret_cast<r_TValue**>(rL + offsets::top), num);
	r_incr_top(rL);
}

void r_lua_pushvalue(std::uintptr_t rL, std::uint32_t idx)
{
        r_setobj2s(*reinterpret_cast<r_TValue**>(rL + offsets::top), r_index2adr(rL, idx));
        r_incr_top(rL);
}

void r_lua_pushinteger(std::uintptr_t rL, std::uint32_t n)
{
	r_setnvalue(*reinterpret_cast<r_TValue**>(rL + offsets::top), cast_num(n));
	r_incr_top(rL);
}

void r_lua_pushlightuserdata(std::uintptr_t rL, void* p) 
{
	r_setpvalue(*reinterpret_cast<r_TValue**>(rL + offsets::top), p);
	r_incr_top(rL);
}

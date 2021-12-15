#include "headers.hpp"
#include <Windows.h>

/* stack -> push */
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

/* state -> to */
std::uint32_t* r_lua_tothread(std::uintptr_t rL, std::uint32_t idx)
{
	r_StkId o = r_index2adr(rL, idx);
	if (o->tt == R_LUA_TTHREAD)
		return &(o)->value.gc->th;
	return NULL;
}

std::double_t r_lua_tonumber(std::uintptr_t rL, std::int32_t idx)
{
	r_TValue* top = r_index2adr(rL, idx);

	std::double_t raw_value = *reinterpret_cast<std::double_t*>(&reinterpret_cast<r_TValue*>(top)->value);
	return r_xor_number(raw_value);
}

std::uint32_t r_lua_tointeger(std::uintptr_t rL, std::uint32_t idx) 
{
	r_TValue* top = r_index2adr(rL, idx);

	const std::double_t raw_value = *reinterpret_cast<std::double_t*>(&(reinterpret_cast<r_TValue*>(top))->value);
	const auto result = static_cast<signed int>(floor(r_xor_number(raw_value)));
	return result;
}

bool r_lua_toboolean(std::uintptr_t rL, std::uint32_t idx)
{
	r_TValue* top = r_index2adr(rL, idx);
	return reinterpret_cast<r_TValue*>(top)->value.b;
}

const void* r_lua_topointer(std::uintptr_t rL, std::uint32_t idx)
{
	r_StkId o = r_index2adr(rL, idx);
	switch (r_ttype(o))
	{
	case R_LUA_TTABLE:
		return &(o)->value.gc->h;
	case R_LUA_TFUNCTION:
		return &(o)->value.gc->cl;
	case R_LUA_TTHREAD:
		return r_lua_tothread(rL, idx);
	case R_LUA_TUSERDATA:
	case R_LUA_TLIGHTUSERDATA:
		return r_lua_touserdata(rL, idx);
	default:
		return NULL;
	}
}

/* L -> GC */
void r_luaC_link(std::uintptr_t rL, std::uint32_t o, r_lu_byte tt)
{
	const auto g = r_G(rL); // getglobalstate
	*reinterpret_cast<std::uint32_t*>(o) = *reinterpret_cast<std::uint32_t*>(g + offsets::g_next);
	*reinterpret_cast<std::uint32_t*>(g + offsets::g_rootgc) = o;
	*reinterpret_cast<r_lu_byte*>(o + offsets::g_marked) = *reinterpret_cast<r_lu_byte*>(g + offsets::g_currentwhite) & 3;
	*reinterpret_cast<r_lu_byte*>(o + offsets::g_ttype) = tt;
}

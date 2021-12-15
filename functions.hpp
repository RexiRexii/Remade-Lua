#include "headers.hpp"
#include <Windows.h>

__forceinline r_TValue* r_index2adr(std::uintptr_t rL, std::int32_t idx)
{
	if (idx > 0)
	{
		r_TValue* o = *reinterpret_cast<r_TValue**>(rL + offsets::base) + (idx - 1);

		if (o >= *reinterpret_cast<r_TValue**>(rL + offsets::top))
			return cast(r_TValue*, r_luaO_nilobject);
		else
			return o;
	}
	else
	{
		return addresses::r_lua_index2adr_slow(rL, idx);
	}
}

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
	const auto result = static_cast<std::int32_t>(floor(r_xor_number(raw_value)));
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

/* state -> other */
void r_lua_xmove(std::uintptr_t from, std::uintptr_t to, std::uint32_t n)
{
	if (from == to) return;
	*reinterpret_cast<std::uintptr_t*>(from + offsets::top) -= (n * sizeof(r_TValue));

	for (auto i = 0; i < n; i++)
	{
		r_setobj2s(*reinterpret_cast<r_TValue**>(to + offsets::top), *reinterpret_cast<r_TValue**>(from + offsets::top) + i);
		r_incr_top(to);
	}
	return;
}

void r_lua_replace(std::uintptr_t rL, std::uint32_t idx)
{
	auto o = r_index2adr(rL, idx);

	if (idx == R_LUA_ENVIRONINDEX)
	{
	//	r_luaL_error(rL, "no calling environment");
		printf("no calling environment\n"); // use r_luaL_error if you want
	}

	r_setobj2s(reinterpret_cast<r_TValue*>(o), reinterpret_cast<r_TValue*>(r_index2adr(rL, -1)));
	r_decr_top(rL);
}

void r_lua_remove(std::uintptr_t rL, std::uint32_t idx)
{
	auto p = r_index2adr(rL, idx);

	while (p < *reinterpret_cast<r_TValue**>(rL + offsets::top))
	{
		r_setobj2s(reinterpret_cast<r_TValue*>(p), reinterpret_cast<r_TValue*>(p + sizeof(r_TValue)));
		p += sizeof(r_TValue);
	}

	r_decr_top(rL);
}

std::uint32_t r_lua_type(std::uintptr_t rL, std::uint32_t idx)
{
	r_TValue* obj = r_index2adr(rL, idx);
	return obj == r_luaO_nilobject ? R_LUA_TNONE : obj->tt;
}

void r_lua_insert(std::uintptr_t rL, std::uint32_t idx)
{
	auto p = r_index2adr(rL, idx);
	for (auto q = *reinterpret_cast<r_TValue**>(rL + offsets::top); q > p; q -= sizeof(r_TValue))
	{
		r_setobj2s(reinterpret_cast<r_TValue*>(q), reinterpret_cast<r_TValue*>(q - sizeof(r_TValue)));
	}

	r_setobj2s(reinterpret_cast<r_TValue*>(p), *reinterpret_cast<r_TValue**>(rL + offsets::top));
}

std::uint32_t r_lua_gettop(std::uintptr_t rL)
{
	return cast_int(*reinterpret_cast<r_TValue**>(rL + offsets::top) - *reinterpret_cast<r_TValue**>(rL + offsets::base));
}

void r_lua_settop(std::uintptr_t rL, std::uint32_t idx)
{
	r_TValue **top = reinterpret_cast<r_TValue**>(rL + offsets::top);
	r_TValue **base = reinterpret_cast<r_TValue**>(rL + offsets::base);
	if (idx >= 0)
	{
		while (*top < *base + idx)
		{
			r_setnilvalue(*top);
			*top++;
		}
		*top = *base + idx;
	}
	else
		*top += idx + 1;
}

void r_lua_pushraw(std::uintptr_t rL, std::uintptr_t obj, int tt)
{
	r_setrawobj2s(rL, obj, tt);
	r_incr_top(rL);
}

std::uint32_t r_lua_getmetatable(std::uintptr_t rL, std::uint32_t idx)
{
	const r_TValue* obj;
	auto mt = 0;
	auto res;
	obj = r_index2adr(rL, idx);

	switch (r_ttype(obj))
	{
	case R_LUA_TTABLE:
	{
		std::uintptr_t v5 = *(DWORD*)obj + 24;
		mt = *(_DWORD*)v5 + 12;
		break;
	}
	case R_LUA_TUSERDATA:
	{
		std::uintptr_t v6 = *(DWORD*)obj + 12;
		mt = v6 ^ *(DWORD*)v6;
		break;
	}
	default:
	{
		mt = *_DWORD*)(4 * *(DWORD*)(obj + 12) + 1304 - (rL + 20) + *(DWORD*)(rL + 20));
		break;
	}
	}
	if (mt == NULL)
		res = 0;
	else
	{
		r_lua_pushraw(rL, mt, R_LUA_TTABLE);
		res = 1;
	}
	return res;
}

std::uint32_t r_lua_setmetatable(std::uintptr_t rL, std::int32_t objindex) // didnt check if this actually works, will remove this comment when i confirm it does
{
	const r_TValue* obj;
	std::uintptr_t mt = 0;
	obj = r_index2adr(rL, objindex);

	switch (r_ttype(obj))
	{
	case R_LUA_TTABLE:
	{
		std::uintptr_t v5 = *(_DWORD*)obj + 24;
		*(_DWORD*)v5 + 12 == mt;
		break;
	}
	case R_LUA_TUSERDATA:
	{
		std::uintptr_t v6 = *(_DWORD*)obj + 12;
		v6 ^ *(_DWORD*)v6 == mt;
		break;
	}
	default:
	{
		*(_DWORD*)(4 * *(_DWORD*)(obj + 12) + 1304 - (rL + 20) + *(_DWORD*)(rL + 20)) = mt;
		break;
	}
	}
	r_decr_top(rL);
	return 1;
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

std::uint32_t r_lua_yield(std::uintptr_t rL, std::uint32_t nresults) {

	if (*reinterpret_cast<std::uintptr_t*>(rL + offsets::l_nccalls) > *reinterpret_cast<std::uintptr_t*>(rL + offsets::l_baseccalls))
		r_luaL_error(rL, "attempt to yield across metamethod/C-call boundary");

	*reinterpret_cast<std::uintptr_t*>(rL + offsets::base) = *reinterpret_cast<std::uintptr_t*>(rL + offsets::top) - 16 * nresults;
	*reinterpret_cast<std::uintptr_t*>(rL + offsets::l_status) = R_LUA_YIELD;
	return -1;
}

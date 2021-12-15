#include <Windows.h>

// Rebasing for addresses
#define aslr(x) (x - 0x400000 + (DWORD)GetModuleHandle(0))

// Lua C-types
typedef BYTE r_lu_byte;
typedef double r_lua_Number;
typedef ptrdiff_t r_lua_Integer;

struct r_GCheader
{
	r_GCObject *next;
	r_lu_byte tt;
	r_lu_byte marked;
	r_lu_byte _pad;
};

union r_GCObject
{
	r_GCheader gch;
	std::uintptr_t ts; /* tstring */
	std::uintptr_t u; /* userdata */
	std::uintptr_t cl; /* closure */
	std::uintptr_t h; /* table */
	std::uintptr_t p; /* pointer */
	std::uintptr_t uv; /* upvalue */
	std::uintptr_t th;  /* thread */
};

typedef union
{
	r_GCObject *gc;
	void *p;
	std::double_t n;
	std::uint32_t b;
} r_Value;

typedef struct r_lua_TValue
{
	r_Value value; 
	r_lua_TValue* ptr; 
	std::uint32_t tt;
} r_TValue;

#define R_LUA_TNONE -1
#define R_LUA_TNIL 0
#define R_LUA_TBOOLEAN 1
#define R_LUA_TNUMBER 2
#define R_LUA_TLIGHTUSERDATA 3
#define R_LUA_TVECTOR 4
#define R_LUA_TSTRING 5
#define R_LUA_TFUNCTION 6
#define R_LUA_TTABLE 7
#define R_LUA_TTHREAD 8
#define R_LUA_TUSERDATA 9

#define R_LUA_REGISTRYINDEX -10000
#define R_LUA_ENVIRONINDEX -10001
#define R_LUA_GLOBALSINDEX -10002

#define r_lua_upvalueindex(i) (R_LUA_GLOBALSINDEX-(i))

#define r_setnilvalue(obj) \
{ \
	r_TValue *i_o = reinterpret_cast<r_TValue*>(obj); \
	i_o->tt = R_LUA_TNIL; \
	i_o->value.p = NULL; \
}

#define r_setnvalue(obj, x) \
{ \
	r_TValue *i_o = reinterpret_cast<r_TValue*>(obj); \
	i_o->value.n = r_xor_number((std::double_t)(x)); \
	i_o->tt = R_LUA_TNUMBER; \
}

#define r_setpvalue(obj, x) \
{ \
	r_TValue *i_o = reinterpret_cast<r_TValue*>(obj); \
	i_o->value.p = x; \
	i_o->tt = R_LUA_TLIGHTUSERDATA; \
}

#define r_setbvalue(obj, x) \
{ \
	r_TValue *i_o = reinterpret_cast<r_TValue*>(obj); \
	i_o->value.b = x; \
	i_o->tt = R_LUA_TBOOLEAN; \
}

#define r_setsvalue(obj, x) \
{ \
	r_TValue *i_o = reinterpret_cast<r_TValue*>(obj); \
	i_o->value.gc = reinterpret_cast<r_GCObject*>(x); \
	i_o->tt = R_LUA_TSTRING; \
}

#define r_setuvalue(obj, x) \
{ \
	r_TValue *i_o = reinterpret_cast<r_TValue*>obj; \
	i_o->value.gc = reinterpret_cast<r_GCObject*>(x); \
	i_o->tt = R_LUA_TUSERDATA; \
}

#define r_setthvalue(obj, x) \
{ \
	r_TValue *i_o = reinterpret_cast<r_TValue*>(obj); \
	i_o->value.gc = reinterpret_cast<r_GCObject*>(x); \
	i_o->tt = R_LUA_TTHREAD; \
}

#define r_setclvalue(obj, x) \
{ \
	r_TValue *i_o = reinterpret_cast<r_TValue*>(obj); \
	i_o->value.gc = reinterpret_cast<r_GCObject*>(x); \
	i_o->tt = R_LUA_TFUNCTION; \
}

#define r_sethvalue(obj, x) \
{ \
	r_TValue *i_o = reinterpret_cast<r_TValue*>(obj); \
	i_o->value.gc = reinterpret_cast<r_GCObject*>(x); \
	i_o->tt = R_LUA_TTABLE; \
}

#define r_setptvalue(obj, x) \
{ \
	r_TValue *i_o = reinterpret_cast<r_TValue*>(obj); \
	i_o->value.gc = reinterpret_cast<r_GCObject*>(x); \
	i_o->tt = R_LUA_TPROTO; \
}

#define r_setobj2s(obj1, obj2) \
{ \
	r_TValue *o1 = (r_TValue*)(obj1), *o2 = (r_TValue*)(obj2); \
	o1->tt = o2->tt; \
	o1->value = o2->value; \
}

#ifndef cast
#define cast(t, exp)	((t)(exp))
#endif

#ifndef cast_to
#define cast_to(t, exp) ((t)(exp))
#endif

#define cast_byte(i)	cast(r_lu_byte, (i))
#define cast_num(i)		cast(r_lua_Number, (i))
#define cast_int(i)		cast(int, (i))

#define r_obj2gco(v)	(cast(r_GCObject *, (v)))

#define r_G(v7) (*(DWORD*)(v7 + offsets::l_G) - (v7 + offsets::l_G)) // THIS CHANGES EVERY UPDATE

namespace offsets
{
	/* OFFSETS */
	const std::uint16_t top = 24;
	const std::uint16_t base = 12;
	const std::uint16_t isC = 7;

	const std::uint16_t ts_len = 12;
	const std::uint16_t ts_str = 20;

	const std::uint16_t g_totalbytes = 56;
	const std::uint16_t g_GCthreshold = 52;
	const std::uint16_t g_frealloc = 12;
	const std::uint16_t g_currentwhite = 20;
	const std::uint16_t g_rootgc = 24;
	const std::uint16_t g_rawhvalue = 20;
	const std::uint16_t g_uvalue = 12;
	const std::uint16_t g_rawuvalue = 16;
	const std::uint16_t g_marked = 4;
	const std::uint16_t g_ttype = 5;
	const std::uint16_t g_next = 32;

	const std::uint16_t l_G = 20;
	const std::uint16_t l_GT = 88;
	const std::uint16_t l_nccalls = 52;
	const std::uint16_t l_baseccalls = 54;
	const std::uint16_t l_status = 7;
	const std::uint16_t l_stackstate = 9;
	const std::uint16_t l_global = 12;

	const std::uint16_t l_ci = 20;
	const std::uint16_t l_base_ci = 40;
	/* END OF OFFSETS */
}

namespace addresses
{
	/* below are the ones that are actively being used. */
	static auto xorconst = aslr(0x35546A0);

	typedef r_TValue* (__fastcall* T_index2adrslow)(std::uintptr_t rL, std::int32_t idx);
	static T_index2adrslow r_lua_index2adr_slow = reinterpret_cast<T_index2adrslow>(aslr(0x1a8fad0));

	typedef std::uint32_t(__fastcall* T_luav_tostring)(std::uintptr_t a1, r_StkId narg);
	static T_luav_tostring r_luaV_tostring = reinterpret_cast<T_luav_tostring>(aslr(0x1B0D770));
}

/* i didnt wanna put misako's xor number here ok */
double r_xor_number(double num)
{
	__int64 xorconst = *reinterpret_cast<__int64*>(&num) ^ *reinterpret_cast<__int64*>(addresses::xorconst);
	return *reinterpret_cast<double*>(&xorconst);
}

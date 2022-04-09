#pragma once
#include <cstdint>
#include <emmintrin.h>
#include <dependencies/roblox/offsets.hpp>

/* struct */
struct r_TValue // since iivillian told me using structs is normal, i will use it for *few funcs only*
{
    union
    {
        std::uintptr_t gc;
        void* p;
        double n;
        int b;
        float v[2];
    } value;

    int extra[1];
    int tt;
};

struct r_CallS // for protected call obv
{
    r_TValue* func;
    int nresults;
};

// offsets
const auto luastate_top = 24;
const auto luastate_base = 12;
const auto luastate_lg = 8;
const auto luastate_activememcat = 4;

const auto globalstate_currentwhite = 20;
const auto globalstate_frealloc = 12;
const auto globalstate_totalbytes = 56;

const auto closure_isc = 3;
const auto closure_env = 12;
const auto closure_preload = 6;
const auto closure_stacksize = 5;
const auto closure_upvals = 32;
const auto closure_nupvals = 4;

const auto closure_debugname = 24;
const auto closure_f = 16;
const auto closure_cont = 20;

const auto tvalue_tt = 12;

const auto gch_marked = 2;
const auto gch_tt = 0;
const auto gch_memcat = 1;

__inline std::uintptr_t r_G(const std::uintptr_t a1)
{
    return *reinterpret_cast<const std::uintptr_t*>(a1 + luastate_lg) - (a1 + luastate_lg);
}

__inline std::uintptr_t r_incr_top(const std::uintptr_t a1)
{
    return *reinterpret_cast<std::uintptr_t*>(a1 + luastate_top) += sizeof(r_TValue);
}

__inline std::uintptr_t r_decr_top(const std::uintptr_t a1)
{
    return *reinterpret_cast<std::uintptr_t*>(a1 + luastate_top) -= sizeof(r_TValue);
}

// addresses
const auto r_luaO_nilobject = aslr(0x3087C80);

const auto pseudo2_address = aslr(0x16D4800);
const auto r_pseudo2addr = reinterpret_cast<r_TValue*(__fastcall*)(std::uintptr_t, std::int32_t)>(pseudo2_address);

const auto xorconst_address = aslr(0x3014E90);
__inline std::double_t r_xor_double(const std::double_t* from)
{
    __m128d xmm_key = _mm_load_pd(reinterpret_cast<const std::double_t*>(xorconst_address));
    __m128d xmm_data = _mm_load_sd(from);
    __m128d xmm_result = _mm_xor_pd(xmm_key, xmm_data);
    return _mm_cvtsd_f64(xmm_result);
}

/*
** basic stack manipulation
*/
__inline r_TValue* r_index2addr(const std::uintptr_t rL, const std::int32_t idx)
{
    if (idx > 0)
    {
        const auto o = *reinterpret_cast<r_TValue**>(rL + luastate_base) + ((idx - 1) * sizeof(r_TValue));

        if (o >= *reinterpret_cast<r_TValue**>(rL + luastate_top))
            *reinterpret_cast<r_TValue**>(r_luaO_nilobject);
        else
            return o;
    }
    else if (idx > -10000)
        return *reinterpret_cast<r_TValue**>(rL + luastate_top) + idx;
    else
        return r_pseudo2addr(rL, idx);
}

__inline std::uintptr_t r_lua_gettop(const std::uintptr_t rL)
{
    return (*reinterpret_cast<r_TValue**>(rL + luastate_top) - *reinterpret_cast<r_TValue**>(rL + luastate_base)) >> 4;
}

__inline void r_lua_settop(const std::uintptr_t rL, const std::int32_t idx)
{
    auto top = *reinterpret_cast<r_TValue**>(rL + luastate_top);
    auto base = *reinterpret_cast<r_TValue**>(rL + luastate_base);

    if (idx >= 0)
    {
        while (top < base + idx)
            top->tt = R_LUA_TNIL;

        top = base + idx;
    }
    else
        top += idx + 1; /* `subtract' index (index is negative) */
}

__inline void r_lua_pushvalue(const std::uintptr_t rL, const std::int32_t idx)
{
    const auto o = r_index2addr(rL, idx);
    const auto top = *reinterpret_cast<const std::uintptr_t*>(rL + luastate_top);

    *reinterpret_cast<double*>(top) = *reinterpret_cast<const double*>(o);
    r_incr_top(rL);
}

__inline void r_lua_remove(const std::uintptr_t rL, const std::int32_t idx)
{
    auto p = r_index2addr(rL, idx);
    const auto top = *reinterpret_cast<r_TValue**>(rL + luastate_top);

    while (++p < top)
        *reinterpret_cast<double*>(p - 1) = *reinterpret_cast<const double*>(p);

    r_decr_top(rL);
}

__inline void r_lua_insert(const std::uintptr_t rL, const std::int32_t idx)
{
    const auto p = r_index2addr(rL, idx);
    const auto top = *reinterpret_cast<r_TValue**>(rL + luastate_top);

    for (auto q = top; q > p; q--)
        *reinterpret_cast<double*>(q) = *reinterpret_cast<const double*>(q - 1);

    *reinterpret_cast<double*>(p) = *reinterpret_cast<const double*>(top);
}

__inline void r_lua_replace(const std::uintptr_t rL, const std::int32_t idx)
{
    const auto o = r_index2addr(rL, idx);
    const auto top = *reinterpret_cast<r_TValue**>(rL + luastate_top);

    if (idx == -10001)
        throw std::exception("no calling environment");

    *reinterpret_cast<double*>(o) = *reinterpret_cast<const double*>(top - 1);
    r_decr_top(rL);
}

__inline void r_lua_xmove(const std::uintptr_t from, const std::uintptr_t to, std::int32_t n)
{
    if (from == to)
        return;

    const auto ttop = *reinterpret_cast<r_TValue**>(to + luastate_top);
    const auto ftop = *reinterpret_cast<r_TValue**>(from + luastate_top) - n;

    for (auto i = 0; i < n; i++)
        *reinterpret_cast<double*>(ttop + i) = *reinterpret_cast<const double*>(ftop + i);

    *reinterpret_cast<r_TValue**>(from + luastate_top) = ftop;
    *reinterpret_cast<r_TValue**>(to + luastate_top) = ttop + n;
}

__inline void r_lua_xpush(const std::uintptr_t from, const std::uintptr_t to, std::int32_t idx)
{
    const auto top = *reinterpret_cast<std::uintptr_t*>(to + luastate_top);
    const auto id = r_index2addr(from, idx);

    *reinterpret_cast<double*>(to + top) = *reinterpret_cast<const double*>(id);
    r_incr_top(to);
}

/*
** access functions (stack -> C)
*/
__inline std::int32_t r_lua_type(const std::uintptr_t rL, const std::int32_t idx)
{
    const auto o = r_index2addr(rL, idx);

    return (o == *reinterpret_cast<r_TValue**>(r_luaO_nilobject)) ? R_LUA_TNONE : o->tt;
}

__inline std::double_t r_lua_tonumber(const std::uintptr_t rL, const std::int32_t idx)
{
    auto top = r_index2addr(rL, idx);

    const auto value = *reinterpret_cast<const std::double_t*>(&(reinterpret_cast<r_TValue*>(top)->value));
    const auto result = r_xor_double(&(value));

    if (result == 0)
        throw std::exception("value is nil");

    return result;
}

__inline std::int32_t r_lua_tointeger(const std::uintptr_t rL, const std::int32_t idx)
{
    auto top = r_index2addr(rL, idx);

    const auto value = *reinterpret_cast<const std::double_t*>(&(reinterpret_cast<r_TValue*>(top)->value));
    const auto result = static_cast<const std::int32_t>(std::floor(r_xor_double(&(value))));
    // std::floor = returns integer value, perfect for what we need it for!
    
    if (result == 0)
        throw std::exception("value is nil");

    return result;
}

__inline bool r_lua_toboolean(const std::uintptr_t rL, const std::int32_t idx)
{
    auto top = r_index2addr(rL, idx);
    const auto result = reinterpret_cast<r_TValue*>(top)->value.b; // wow! even tho I could simply do this without the struct, I chose to use it! Little difference wouldnt hurt ;)

    if (result == 0)
        throw std::exception("value is nil");

    return result;
}

__inline void* r_lua_touserdata(const std::uintptr_t rL, const std::int32_t idx)
{
    const auto o = r_index2addr(rL, idx);

    switch (o->tt)
    {
    case R_LUA_TLIGHTUSERDATA:
        return *reinterpret_cast<void**>(o);
    case R_LUA_TUSERDATA:
        return reinterpret_cast<void*>(*reinterpret_cast<const std::uintptr_t*>(o) + 16u);
    default:
        return 0;
    }
}

__inline std::int32_t r_luaD_call(const std::uintptr_t rL, r_TValue* func, const std::int32_t nresults)
{
    const auto ptr = std::make_unique<r_CallS>();
    const auto ud = ptr.get();

    ud->func = func;
    ud->nresults = nresults;

    std::int32_t result = 0;

    __asm
    {
        push edx;
        push eax;
        push ecx;

        mov edx, [luad_precall_address];
        mov ecx, [rL];
        mov eax, [ud];
        push eax;
        call[luad_rawrunprotected_address];
        mov[result], eax;
        add esp, 4;

        pop ecx;
        pop eax;
        pop edx;
    }
    return result;
}

__inline void r_lua_call(const std::uintptr_t rL, const std::int32_t nargs, const std::int32_t nresults)
{
    auto func = *reinterpret_cast<r_TValue**>(rL + luastate_top) - (nargs + 1);

    r_luaD_call(rL, func, nresults);
}

__inline void r_f_call(const std::uintptr_t rL, void* ud)
{
    auto c = reinterpret_cast<r_CallS*>(ud);
    r_luaD_call(rL, c->func, c->nresults);
}

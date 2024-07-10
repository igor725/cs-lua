#include <core.h>
#include <cpe.h>
#include "luascript.h"
#include "luacolor.h"

static CPEParticle *checkparticle(lua_State *L, int idx) {
	return luaL_checkudata(L, idx, CSSCRIPTS_MPARTICLE);
}

static int particle_undefine(lua_State *L) {
	lua_pushboolean(L, CPE_UndefineParticlePtr(
		checkparticle(L, 1)
	));
	return 1;
}

static const luaL_Reg particlemeta[] = {
	{"__gc", particle_undefine},

	{NULL, NULL}
};

static int particle_create(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	lua_checkstack(L, 18);

	CPEParticle *part = lua_newuserdata(L, sizeof(CPEParticle));
	luaL_setmetatable(L, CSSCRIPTS_MPARTICLE);
	int top = lua_gettop(L);

	if(scr_checktabfield(L, 1, "uv", LUA_TTABLE)) {
			for(int i = 0; i < 4; i++) {
				lua_rawgeti(L, -1 - i, i + 1);
				if(!lua_isnumber(L, -1))
					luaL_error(L, "Particle UV has invalid format", i + 1);
				((cs_byte *)&part->rec)[i] = (cs_byte)lua_tointeger(L, -1);
			}
	}
	if(scr_checktabfieldud(L, 1, "tintcolor", CSSCRIPTS_MCOLOR))
		part->tintCol = *scr_tocolor3(L, -1);
	if(scr_checktabfield(L, 1, "framecount", LUA_TNUMBER))
		part->frameCount = (cs_byte)lua_tointeger(L, -1);
	if(scr_checktabfield(L, 1, "particlecnt", LUA_TNUMBER))
		part->particleCount = (cs_byte)lua_tointeger(L, -1);
	if(scr_checktabfield(L, 1, "size", LUA_TNUMBER))
		part->size = (cs_byte)lua_tointeger(L, -1);
	if(scr_checktabfield(L, 1, "sizevar", LUA_TNUMBER))
		part->sizeVariation = (cs_float)lua_tonumber(L, -1);
	if(scr_checktabfield(L, 1, "spread", LUA_TNUMBER))
		part->spread = (cs_float)lua_tonumber(L, -1);
	if(scr_checktabfield(L, 1, "speed", LUA_TNUMBER))
		part->speed = (cs_float)lua_tonumber(L, -1);
	if(scr_checktabfield(L, 1, "gravity", LUA_TNUMBER))
		part->gravity = (cs_float)lua_tonumber(L, -1);
	if(scr_checktabfield(L, 1, "lifetime", LUA_TNUMBER))
		part->baseLifetime = (cs_float)lua_tonumber(L, -1);
	if(scr_checktabfield(L, 1, "lifetimevar", LUA_TNUMBER))
		part->lifetimeVariation = (cs_float)lua_tonumber(L, -1);
	if(scr_checktabfield(L, 1, "colflags", LUA_TNUMBER))
		part->collideFlags = (cs_byte)lua_tointeger(L, -1);
	if(scr_checktabfield(L, 1, "fullbright", LUA_TBOOLEAN))
		part->fullBright = scr_toboolean(L, -1);

	lua_settop(L, top);
	return 1;
}

static int particle_define(lua_State *L) {
	lua_pushboolean(L, CPE_DefineParticle(
		(cs_byte)luaL_checkinteger(L, 1),
		checkparticle(L, 2)
	));
	return 1;
}

static int particle_freeid(lua_State *L) {
	cs_int16 id = -1;
	for(cs_byte i = 0; i < CPE_MAX_PARTICLES; i++) {
		if(!CPE_IsParticleDefined(i)) {
			id = i;
			break;
		}
	}
	lua_pushinteger(L, (lua_Integer)id);
	return 1;
}

static const luaL_Reg particlelib[] = {
	{"create", particle_create},
	{"define", particle_define},
	{"undefine", particle_undefine},
	{"freeid", particle_freeid},

	{NULL, NULL}
};

int scr_libfunc(particle)(lua_State *L) {
	scr_createtype(L, CSSCRIPTS_MPARTICLE, particlemeta);
	luaL_newlib(L, particlelib);
	return 1;
}

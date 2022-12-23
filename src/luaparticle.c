#include <core.h>
#include <cpe.h>
#include "luascript.h"
#include "luacolor.h"

static CPEParticle *checkparticle(scr_Context *L, int idx) {
	return scr_checkmemtype(L, idx, CSSCRIPTS_MPARTICLE);
}

static int particle_undefine(scr_Context *L) {
	scr_pushboolean(L, CPE_UndefineParticlePtr(
		checkparticle(L, 1)
	));
	return 1;
}

static const scr_RegFuncs particlemeta[] = {
	{"__gc", particle_undefine},

	{NULL, NULL}
};

static int particle_create(scr_Context *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	scr_stackcheck(L, 18);

	CPEParticle *part = scr_allocmem(L, sizeof(CPEParticle));
	scr_setmemtype(L, CSSCRIPTS_MPARTICLE);
	int top = scr_stacktop(L);

	if(scr_checktabfield(L, 1, "uv", LUA_TTABLE)) {
			for(int i = 0; i < 4; i++) {
				scr_rawgeti(L, -1 - i, i + 1);
				if(!scr_isnumber(L, -1))
					scr_fmterror(L, "Particle UV has invalid format", i + 1);
				((cs_byte *)&part->rec)[i] = (cs_byte)scr_tointeger(L, -1);
			}
	}
	if(scr_checktabfieldud(L, 1, "tintcolor", CSSCRIPTS_MCOLOR))
		part->tintCol = *scr_tocolor3(L, -1);
	if(scr_checktabfield(L, 1, "framecount", LUA_TNUMBER))
		part->frameCount = (cs_byte)scr_tointeger(L, -1);
	if(scr_checktabfield(L, 1, "particlecnt", LUA_TNUMBER))
		part->particleCount = (cs_byte)scr_tointeger(L, -1);
	if(scr_checktabfield(L, 1, "size", LUA_TNUMBER))
		part->size = (cs_byte)scr_tointeger(L, -1);
	if(scr_checktabfield(L, 1, "sizevar", LUA_TNUMBER))
		part->sizeVariation = (cs_float)scr_tonumber(L, -1);
	if(scr_checktabfield(L, 1, "spread", LUA_TNUMBER))
		part->spread = (cs_float)scr_tonumber(L, -1);
	if(scr_checktabfield(L, 1, "speed", LUA_TNUMBER))
		part->speed = (cs_float)scr_tonumber(L, -1);
	if(scr_checktabfield(L, 1, "gravity", LUA_TNUMBER))
		part->gravity = (cs_float)scr_tonumber(L, -1);
	if(scr_checktabfield(L, 1, "lifetime", LUA_TNUMBER))
		part->baseLifetime = (cs_float)scr_tonumber(L, -1);
	if(scr_checktabfield(L, 1, "lifetimevar", LUA_TNUMBER))
		part->lifetimeVariation = (cs_float)scr_tonumber(L, -1);
	if(scr_checktabfield(L, 1, "colflags", LUA_TNUMBER))
		part->collideFlags = (cs_byte)scr_tointeger(L, -1);
	if(scr_checktabfield(L, 1, "fullbright", LUA_TBOOLEAN))
		part->fullBright = scr_toboolean(L, -1);

	scr_stackset(L, top);
	return 1;
}

static int particle_define(scr_Context *L) {
	scr_pushboolean(L, CPE_DefineParticle(
		(cs_byte)scr_checkinteger(L, 1),
		checkparticle(L, 2)
	));
	return 1;
}

static int particle_freeid(scr_Context *L) {
	cs_int16 id = -1;
	for(cs_byte i = 0; i < CPE_MAX_PARTICLES; i++) {
		if(!CPE_IsParticleDefined(i)) {
			id = i;
			break;
		}
	}
	scr_pushinteger(L, (scr_Integer)id);
	return 1;
}

static const scr_RegFuncs particlelib[] = {
	{"create", particle_create},
	{"define", particle_define},
	{"undefine", particle_undefine},
	{"freeid", particle_freeid},

	{NULL, NULL}
};

int scr_libfunc(particle)(scr_Context *L) {
	scr_createtype(L, CSSCRIPTS_MPARTICLE, particlemeta);
	scr_newlib(L, particlelib);
	return 1;
}

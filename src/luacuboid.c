#include <core.h>
#include <client.h>
#include <cpe.h>
#include "luascript.h"
#include "luaclient.h"
#include "luacuboid.h"
#include "luacolor.h"
#include "luavector.h"

typedef struct _LuaCuboid {
	cs_bool released;
	Client *client;
	CPECuboid *cub;
} LuaCuboid;

static void setcube(scr_Context *L, LuaCuboid *luacub) {
	scr_stackpush(L, -4);
	scr_rawseti(L, -2, Cuboid_GetID(luacub->cub));
}

void scr_newcubref(scr_Context *L, Client *client, CPECuboid *cub) {
	if(cub == NULL) {
		scr_pushnull(L);
		return;
	}

	LuaCuboid *luacub = scr_allocmem(L, sizeof(LuaCuboid));
	scr_setmemtype(L, CSSCRIPTS_MCUBOID);
	luacub->released = false;
	luacub->client = client;
	luacub->cub = cub;

	scr_gettabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCUBOIDS);
	scr_pushclient(L, client);
	scr_stackpush(L, -1); // Отправляем клиента дважды в стек, может пригодиться
	scr_getfromtable(L, -3);
	if(scr_isnull(L, -1)) {
		scr_stackpop(L, 1);
		scr_newntable(L, CPE_MAX_CUBOIDS, 0); // Таблица кубов
		scr_newntable(L, 0, 1); // Метатаблица для таблицы кубов (TODO: Сделать её общей для всех??)
		// Делаем, чтобы сборщик мусора не учитывал ссылки внутри таблицы
		scr_pushstring(L, "v");
		scr_settabfield(L, -2, "__mode");
		lua_setmetatable(L, -2);
		// Заносим куб в новоиспечённую таблицу
		setcube(L, luacub);
		scr_settotable(L, -3); // Сохраняем таблицу кубов клиента в cscuboids
		scr_stackpop(L, 1);
		return;
	}

	setcube(L, luacub);
	scr_stackpop(L, 3); // Убираем из стека лишнего клиента и таблицы
}

void scr_clearcuboids(scr_Context *L, Client *client) {
	scr_pushclient(L, client); // Не хочу два раза вызвывать эту довольно жирную функцию
	scr_gettabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCUBOIDS);
	scr_stackpush(L, -2);
	scr_getfromtable(L, -2); // Получаем таблицу кубоидов клиента

	if(scr_isnull(L, -1)) { // Если таблица не существует, прекращаем выполнение
		scr_stackpop(L, 2);
		return;
	}

	// Проходимся по таблице и удаляем найденные кубоиды
	for(scr_Integer i = 0; i < CPE_MAX_CUBOIDS; i++) {
		scr_pushinteger(L, i);
		scr_getfromtable(L, -2);
		if(scr_isnull(L, -1)) { // Кубоид с указанным id не существует в таблице кубоидов
			scr_stackpop(L, 1);
			continue;
		}
		scr_gettabfield(L, -1, "remove"); // Вытаскиваем у кубоида функцию remove
		if(scr_isfunc(L, -1)) { // Проверяем, функция ли она вообще
			scr_stackpush(L, -2); // Отправляем сам кубоид в качестве аргумента функции
			scr_unprotectedcall(L, 1, 0);
		} else scr_fmterror(L, "How it possible? :/"); // Сюда, по идее мы не должны дойти ни при каких условиях
		scr_stackpop(L, 1);
	}

	scr_stackpush(L, -3); // Пушим клиента
	scr_pushnull(L);
	scr_settotable(L, -4); // Убираем таблицу кубоидов клиента из cscuboids
	scr_stackpop(L, 3); // Выкидываем из стека клиента и две таблицы
}

static LuaCuboid *scr_checkcuboid(scr_Context *L, int idx) {
	LuaCuboid *luacub = scr_checkmemtype(L, idx, CSSCRIPTS_MCUBOID);
	scr_argassert(L, !luacub->released, 1, "Cuboid removed");
	return luacub;
}

static LuaCuboid *scr_tocuboid(scr_Context *L, int idx) {
	LuaCuboid *luacub = scr_testmemtype(L, idx, CSSCRIPTS_MCUBOID);
	if(luacub && luacub->released) luacub = NULL;
	return luacub;
}

static int meta_setpoints(scr_Context *L) {
	Cuboid_SetPositions(
		scr_checkcuboid(L, 1)->cub,
		*scr_checkshortvector(L, 2),
		*scr_checkshortvector(L, 3)
	);
	return 0;
}

static int meta_setcolor(scr_Context *L) {
	Cuboid_SetColor(
		scr_checkcuboid(L, 1)->cub,
		*scr_checkcolor4(L, 2)
	);
	return 0;
}

static int meta_getsize(scr_Context *L) {
	scr_pushinteger(L, (scr_Integer)Cuboid_GetSize(
		scr_checkcuboid(L, 1)->cub
	));
	return 1;
}

static int meta_getpoints(scr_Context *L) {
	CPECuboid *cub = scr_checkcuboid(L, 1)->cub;
	SVec *vs = scr_toshortvector(L, 2);
	SVec *ve = scr_toshortvector(L, 3);

	if(!vs) {
		LuaVector *lvs = scr_newvector(L);
		lvs->type = LUAVECTOR_TSHORT;
		vs = &lvs->value.s;
	}

	if(!ve) {
		LuaVector *lve = scr_newvector(L);
		lve->type = LUAVECTOR_TSHORT;
		ve = &lve->value.s;
	}

	Cuboid_GetPositions(cub, vs, ve);
	return 2;
}

static int meta_update(scr_Context *L) {
	LuaCuboid *luacub = scr_checkcuboid(L, 1);
	Client_UpdateSelection(luacub->client, luacub->cub);
	return 0;
}

static int meta_remove(scr_Context *L) {
	LuaCuboid *luacub = scr_tocuboid(L, 1);

	if(luacub) {
		luacub->released = true;
		Client_RemoveSelection(luacub->client, luacub->cub);
		scr_gettabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCUBOIDS);
		scr_pushclient(L, luacub->client);
		scr_getfromtable(L, -2);
		scr_pushinteger(L, Cuboid_GetID(luacub->cub));
		scr_pushnull(L);
		scr_settotable(L, -3);
		scr_stackpop(L, 1);
	}

	return 0;
}

static const scr_RegFuncs cuboidmeta[] = {
	{"setpoints", meta_setpoints},
	{"setcolor", meta_setcolor},

	{"getsize", meta_getsize},
	{"getpoints", meta_getpoints},

	{"update", meta_update},
	{"remove", meta_remove},

	{"__gc", meta_remove},

	{NULL, NULL}
};

void luainit_cuboid(scr_Context *L) {
	scr_newntable(L, 0, MAX_CLIENTS);
	scr_settabfield(L, LUA_REGISTRYINDEX, CSSCRIPTS_RCUBOIDS);
	scr_createtype(L, CSSCRIPTS_MCUBOID, cuboidmeta);
}

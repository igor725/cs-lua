setInfo {
	version = 1,
	hutreload = true,
	home = 'https://github.com/igor725/cs-lua/blob/main/scripts/chatexec.lua'
}

local LUA_COMMAND = 'LuaRun'
local LUA_COMMAND_DESC = 'Execute Lua script'
local LUA_COMMAND_EMPTY = '&cAttempt to execute an empty string'
local LUA_COMMAND_EXEC_WARN = '%s executed lua script: %s'
local LUA_COMMAND_EXEC_RTERR = '&cRuntime error: %s'
local LUA_COMMAND_EXEC_SUCC = '&aScript executed successfully'
local LUA_COMMAND_EXEC_SYNT = '&cSyntax error: %s'
local LUA_COMMAND_EXEC_OUT = '&aOutput&f: '

local executor = load
if _VERSION == 'Lua 5.1' and not jit then
	executor = loadstring
end

local function runScript(cl, script)
	if not script or #script == 0 then
		return LUA_COMMAND_EMPTY
	end

	_G.self = cl
	local chunk, err = executor(script, 'chatexec')

	if chunk then
		if cl then
			log.warn(LUA_COMMAND_EXEC_WARN, cl:getname(), script)
		end

		local out = {pcall(chunk)}
		_G.self = nil

		if not table.remove(out, 1) then
			return (LUA_COMMAND_EXEC_RTERR):format(out[1])
		end

		for i = 1, #out do
			out[i] = tostring(out[i])
		end

		if #out > 0 then
			return LUA_COMMAND_EXEC_OUT .. table.concat(out, ' ')
		end

		return LUA_COMMAND_EXEC_SUCC
	else
		return (LUA_COMMAND_EXEC_SYNT):format(err)
	end
end

function onStart()
	setmetatable(_G, {
		__index = function(self, key)
			local u = rawget(self, 'client').getbyname(key)
			if u then return u end
			return rawget(self, key)
		end
	})

	command.add(LUA_COMMAND, LUA_COMMAND_DESC, CMDF_OP, runScript)
end

function onMessage(cl, _, text)
	if not cl:isop() then
		return true
	end

	if text:find('^>') then
		text = text:gsub('^(>=?)', function(s)
			if s == '>=' then
				return 'return '
			end

			return ''
		end)

		cl:chat(runScript(cl, text))
		return false
	end
end

function onStop()
	command.remove(LUA_COMMAND)
end

preReload = onStop

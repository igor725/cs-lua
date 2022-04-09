allowHotReload(true)

local function runScript(cl, script)
	if not script or #script == 0 then
		return '&cTrying to execute empty string'
	end

	_G.self = cl
	local chunk, err = load(script, 'chatexec')

	if chunk then
		if cl then
			log.warn('%s executed lua script: %s', cl:getname(), script)
		end

		local out = {pcall(chunk)}
		_G.self = nil

		if not table.remove(out, 1) then
			return ('&cRuntime error: %s'):format(out[1])
		end

		for i = 1, #out do
			out[i] = tostring(out[i])
		end

		if #out > 0 then
			return '&aOutput&f: ' .. table.concat(out, ' ')
		end

		return '&aScript executed successfully'
	else
		return ('&cSyntax error: %s'):format(err)
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

	command.add('luarun', 'Execute Lua script', CMDF_OP, runScript)
end

function onMessage(cl, type, text)
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
	command.remove('luarun')
end

preReload = onStop

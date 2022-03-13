allowHotReload(true)

function onStart()
	setmetatable(_G, {
		__index = function(self, key)
			local u = rawget(self, 'client').getbyname(key)
			if u then return u end
			return rawget(self, key)
		end
	})
end

function onMessage(cl, type, text)
	if cl:isop() then
		if text:find('^>') then
			text = text:gsub('^(>=?)', function(s)
				if s == '>=' then
					return 'return '
				end

				return ''
			end)

			_G.self = cl
			local chunk, err = load(text, 'chatexec')

			if chunk then
				log.warn('%s executed lua script: %s', cl:getname(), text)
				local out = {pcall(chunk)}
				_G.self = nil

				if not table.remove(out, 1) then
					cl:chat(('&cRuntime error: %s'):format(out[1]))
					return false
				end

				for i = 1, #out do
					out[i] = tostring(out[i])
				end

				if #out > 0 then
					cl:chat('&aOutput&f: ' .. table.concat(out, ' '))
				else
					cl:chat('&aScript executed successfully')
				end
			else
				cl:chat(('&cSyntax error: %s'):format(err))
			end

			return false
		end
	end
end

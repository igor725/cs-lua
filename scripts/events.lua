allowHotReload(true)

function onStart()
	badwords = {'(fuck)', '(bitch)', '(dick)', '(cunt)'}
end

function onConnect()
	-- 50% chance of connection rejection
	return math.random(0, 100) > 50
end

function onMessage(sender, mtype, msg)
	for i = 1, #badwords do
		msg = msg:gsub(badwords[i], '***')
	end

	return mtype, msg
end

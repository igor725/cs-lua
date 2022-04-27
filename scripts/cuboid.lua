allowHotReload(true)

function onStart()
	cuboids = {}
	math.randomseed(os.time())

	command.add('MkCuboid', 'Spawns the cuboid', CMDF_CLIENT, function(caller)
		local cuboid = cuboids[caller] or caller:newcuboid()
		if not cuboid then
			return '&cCuboids limit exceeded'
		end

		local vec = caller:getposition():toshort()
		local dvec = vector.short(
			math.random(4, 15),
			math.random(4, 15),
			math.random(4, 15)
		)
		local startp, endp = vec - dvec, vec + dvec
		local color = color.c4(
			math.random(0, 255),
			math.random(0, 255),
			math.random(0, 255),
			math.random(40, 150)
		)

		cuboid:setpoints(startp, endp)
		cuboid:setcolor(color)
		cuboid:update()

		cuboids[caller] = cuboid
		return '&aCuboid spawned!'
	end)

	command.add('DelCuboid', 'Removes the cuboid', CMDF_CLIENT, function(caller)
		if cuboids[caller] then
			cuboids[caller]:remove()
			cuboids[caller] = nil
			return '&aCuboid removed!'
		end

		return '&cYou don\'t have any cuboids'
	end)
end

function preReload()
	command.remove('MkCuboid')
	command.remove('DelCuboid')
end

onStop = preReload

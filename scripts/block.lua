function onStart()
	myblock = block.define{
		name = 'My non-ext block',
		extended = false,
		fallback = 1, -- Stone block
		params = {
			solidity = BDSOL_SWIM,
			movespeed = 200,
			toptex = 14,
			sidetex = 14,
			bottomtex = 14,
			transmitsLight = false,
			walksound = BDSND_WOOD,
			fullbright = false,
			shape = 16,
			drawtype = BDDRW_TRANSLUCENT,
			fogdensity = 20,
			fogr = 100,
			fogg = 20,
			fogb = 10
		}
	}

	myextblock = block.define{
		name = 'My ext block',
		extended = true,
		fallback = 2, -- Grass block
		params = {
			solidity = BDSOL_SWIM,
			movespeed = 200,
			toptex = 51,
			lefttex = 14,
			righttex = 51,
			fronttex = 14,
			backtex = 14,
			bottomtex = 51,
			transmitsLight = false,
			walksound = BDSND_GLASS,
			fullbright = false,
			minx = 1, miny = 1, minz = 1,
			maxx = 14, maxy = 14, maxz = 14,
			drawtype = BDDRW_TRANSLUCENT,
			fogdensity = 20,
			fogr = 100,
			fogg = 20,
			fogb = 10
		}
	}
end

function onWorldAdded(world)
	myblock:addtoworld(world, 67)
	myextblock:addtoworld(world, 68)
end

allowHotReload(true)
CTTT_FLAGS = bit.bor(CMDF_CLIENT, CMDF_OP)
FCUBE_VECTOR = vector.short(4 * 3, 4 * 3, 4 * 3)
ZERO_VECTOR = vector.short(0, 0, 0)
INVALID_VECTOR = vector.short(-1, -1, -1)
CUBOID_COLORS = {
	[true] = color.c4(20, 150, 20, 120),
	[false] = color.c4(150, 20, 20, 120)
}
MODE_VECTORS = {
	vector.short(1, 1,  0),
	vector.short(1, 0,  1),
	vector.short(0, 1, 1)
}

local function testBlocks(w, p1, p2)
	local tmp = vector.short()
	for x = p1.x, p2.x do
		for y = p1.y, p2.y do
			for z = p1.z, p2.z do
				tmp:set(x, y, z)
				if w:getblock(tmp) ~= block.AIR then
					return false
				end
			end
		end
	end

	return true
end

local function calcCuboid(tmp, wrld)
	local dims = wrld:getdimensions()
	local p1 = tmp.pos + vector.short(0, 1, 0)
	local p2 = p1 + FCUBE_VECTOR * tmp.dir
	tmp.cuboid:setpoints(p1, p2)
	tmp.havespace = p1 > ZERO_VECTOR and p2 < dims

	if tmp.havespace then
		tmp.havespace = testBlocks(wrld, p1, p2)
	end

	tmp.cuboid:setcolor(CUBOID_COLORS[tmp.havespace])
	tmp.cuboid:update()
end

-- TODO: Использовать BulkBlockUpdate вместо world:setblocknat(...)

local function drawField(wrld, p1, p2, id)
	local bp = vector.short()
	for x = p1.x, p2.x do
		for y = p1.y, p2.y do
			for z = p1.z, p2.z do
				bp:set(x, y, z)
				wrld:setblocknat(bp, id)
			end
		end
	end
end

local function drawZero(wrld, p, dir, id)
	for i = 0, 2 do
		for j = 0, 2 do
			if i ~= 1 or j ~= 1 then
				if dir.x == 0 then
					wrld:setblocknat(p + vector.short(j, i, j) * dir, id)
				else
					wrld:setblocknat(p + vector.short(i, j, j) * dir, id)
				end
			end
		end
	end
end

local function drawCross(wrld, p, dir, id)
	for i = 0, 2 do
		wrld:setblocknat(p + dir * i, id)
		if dir.x == 0 then
			wrld:setblocknat(p + dir * vector.short(i, 2 - i, i), id)
		else
			wrld:setblocknat(p + dir * vector.short(2 - i, i, i), id)
		end
	end
end

local function messageTo(cl, msg)
	cl:chat(('[&aTTT&f] %s'):format(msg))
end

function onStart()
	Temp = {}
	Games = {
		meta = {
			startGame = function(self)
				if #self.players < 2 or self.started then
					return false
				end
				self.started = true
				self:msgBroadcast('Game started!')
				return true
			end,
			isPlaying = function(self, cl)
				return self.players[1] == cl or
				self.players[2] == cl
			end,
			forceEnd = function(self)
				self.started = false
				self:msgBroadcast('Game over')
				drawField(self.w, self.p1, self.p2, block.AIR)
				for i = #Games, 1, -1 do
					if Games[i] == self then
						table.remove(Games, i)
						return
					end
				end
			end,
			addPlayer = function(self, cl)
				if #self.players > 1 or self:isPlaying(cl) or self.started then
					return false
				end

				if self.private and #self.players > 0 then
					if self.private ~= cl then
						messageTo(cl, '&cThis is a private game and you were not invited. GO AWAY!')
						return false
					end
				end

				table.insert(self.players, cl)
				if not self:startGame() then
					messageTo(cl, 'Waiting for a second player...')
				else
					self:switchTurn()
				end

				return true
			end,
			checkWin = function(self)
				local diagWinner
				for i = 0, 1 do -- Проверяем диагональные победы
					local win = true

					for j = 0, 2 do
						local x = math.abs(2 * i - j)
						if j == 0 then
							diagWinner = self.field[j][x]
							if diagWinner == 0 then
								win = false
								break
							end
						end

						if diagWinner ~= self.field[j][x] then
							win = false
							break
						end
					end

					if win then
						return true
					end
				end

				for i = 0, 2 do -- Проверяем строчные/столбцовые победы
					local winner = self.field[i][0]
					local winnerRow, winnerCol = winner, winner

					for j = 0, 2 do
						if self.field[i][j] ~= winner then
							winnerRow = 0
						end

						if self.field[j][i] ~= winner then
							winnerCol = 0
						end
					end

					if winnerRow + winnerCol > 0 then
						return true
					end
				end

				return false
			end,
			makeClick = function(self, cl, fp)
				if not self.started then
					return
				end
				local v = (self.cross and 1) or 2
				if self.players[v] ~= cl then
					return
				end

				local field = self.field
				local dir = self.dir

				if dir.y == 0 then -- Поле валяется
					if field[fp.z][fp.x] ~= 0 then
						return
					end

					field[fp.z][fp.x] = v
				elseif dir.x == 0 then -- Поле развёрнуто по оси Z
					if field[fp.y][fp.z] ~= 0 then
						return
					end

					field[fp.y][fp.z] = v
				elseif dir.z == 0 then -- Поле развёрнуто по оси X
					if field[fp.y][fp.x] ~= 0 then
						return
					end

					field[fp.y][fp.x] = v
				end

				bc = self.p1 + fp * 4 + dir

				if self.cross then
					drawCross(self.w, bc, dir, block.BLACK)
				else
					drawZero(self.w, bc, dir, block.GRAY)
				end

				if not self:checkWin() then
					self:switchTurn()
				else
					self:msgBroadcast(('%s - winner!'):format(self.cross and 'X' or 'O'))
					self:forceEnd()
				end
			end,
			switchTurn = function(self)
				self.cross = not self.cross
				messageTo(self.players[(self.cross and 1) or 2], 'Your turn!')
			end,
			msgBroadcast = function(self, msg)
				for i = 1, #self.players do
					messageTo(self.players[i], msg)
				end
			end,
			setPrivate = function(self, t)
				self.private = t
			end
		},
		create = function(self, cl, wrld, p1, p2, dir)
			local game = setmetatable({
				p1 = p1, p2 = p2,
				started = false,
				players = {},
				cross = false,
				dir = dir,
				w = wrld,
				field = {
					[0] = {[0] = 0, 0, 0},
					[1] = {[0] = 0, 0, 0},
					[2] = {[0] = 0, 0, 0}
				}
			}, self.meta)
			table.insert(self, game)
			game:addPlayer(cl)
			return game
		end
	}
	Games.meta.__index = Games.meta

	command.add('TicTacToe', 'Creates tictactoe game', CTTT_FLAGS, function(caller, args)
		local ingame, game = false, nil
		for i = 1, #Games do
			if Games[i]:isPlaying(caller) then
				game = Games[i]
				ingame = true
				break
			end
		end
		if ingame and args == 'cancel' then
			game:forceEnd()
			return
		end
		if ingame or Temp[caller] then
			return '&cThe game is already in progress'
		end

		local cub = caller:newcuboid()
		if not cub then
			return '&cFailed to create cuboid object'
		end

		local priv
		if args then
			priv = args:match('^(.+)%s?')
			if priv then
				priv = client.getbyname(priv)
				if not priv then
					return '&cPlayer not found'
				end
			end
		end

		Temp[caller] = {
			private = priv,
			cuboid = cub,
			mode = 1
		}

		return '&aEntering field creation state...'
	end)
end

function onPlayerClick(cl, params)
	local tmp = Temp[cl]
	local cpos = params.position
	if not params.action then
		return
	end

	if tmp then
		local wrld = cl:getworld()
		if params.button == 0 then -- ЛКМ
			if not tmp.havespace then
				messageTo(cl, '&cTicTacToe field cannot be placed here!')
				return
			end
			local p1, p2 = tmp.cuboid:getpoints()
			p2 = p2 - vector.short(1, 1, 1)
			drawField(wrld, p1, p2, block.WHITE)
			tmp.cuboid:remove()
			Temp[cl] = nil
			local game = Games:create(cl, wrld, p1, p2, tmp.dir)
			if tmp.private then game:setPrivate(tmp.private) end
		elseif params.button == 2 then -- СКМ
			if not tmp.pos then return end
			tmp.mode = tmp.mode + 1
			if tmp.mode > #MODE_VECTORS then
				tmp.mode = 1
			end
			tmp.dir = MODE_VECTORS[tmp.mode]
			calcCuboid(tmp, wrld)
		elseif params.button == 1 then -- ПКМ
			if cpos ~= INVALID_VECTOR then
				tmp.pos = cpos
				tmp.dir = MODE_VECTORS[tmp.mode]
				calcCuboid(tmp, wrld)
			end
		end

		return
	end

	if params.button ~= 2 then
		for i = 1, #Games do
			local game = Games[i]
			if cpos >= game.p1 and cpos <= game.p2 then
				if game:isPlaying(cl) then
					game:makeClick(cl, (cpos - game.p1) / 4)
				else
					game:addPlayer()
				end
			end
		end
	end
end

function onBlockPlace(cl, pos, id)
	if Temp[cl] then return false end

	for i = 1, #Games do
		local game = Games[i]
		if pos >= game.p1 and pos <= game.p2 then
			game:addPlayer(cl)
			return false
		end
	end

	return true
end

onBlockDestroy = onBlockPlace

function onDisconnect(cl)
	for i = #Games, 1, -1 do
		local game = Games[i]
		if game:isPlaying(cl) then
			game:forceEnd()
		end
	end
end

function preReload()
	command.remove('TicTacToe')
	for cl, data in pairs(Temp) do
		data.cuboid:remove()
	end
	for i = #Games, 1, -1 do
		Games[i]:forceEnd()
	end
end

onStop = preReload

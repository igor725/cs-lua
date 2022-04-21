local vf = vector.float

function onStart()
	mdl = model.create{
		name = 'TestModel',
		flags = 0x00,
		nameY = 1.2,
		eyeY = 0.7,
		collideBox = vf(1.2, 0.8, 1.2),
		clickMin = vf(-0.4, 0.0, -0.4),
		clickMax = vf(0.4, 0.8, 0.4),
		uScale = 32, vScale = 64,
		parts = {
			{
				flags = 0x00,
				minCoords = vf(0.1, 0.1, 0.1),
				maxCoords = vf(0.9, 0.9, 0.9),
				uvs = {
					{0, 0, 8, 8},
					{8, 8, 16, 16},
					{16, 16, 24, 32},
					{24, 16, 32, 32},
					{32, 16, 40, 32},
					{40, 16, 48, 32}
				},
				rotOrigin = vf(0.5, 0.9, 0.5),
				rotAngles = vf(45.0, 0.0, 0.0),
				anims = {
					{flags = 2, args = {5, 2, 0, 0}},
					{flags = 0, args = {0, 0, 0, 0}},
					{flags = 0, args = {0, 0, 0, 0}},
					{flags = 0, args = {0, 0, 0, 0}}
				}
			}
		}
	}

	model.define(1, mdl)
end

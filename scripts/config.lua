function onStart()
	mycfg = config.new{
		name = 'mycfg.cfg',
		items = {
			{
				name = 'my-key-int',
				type = CONFIG_TYPE_INT,
				comment = 'Signed int key',
				default = -1
			},
			{
				name = 'my-key-str',
				type = CONFIG_TYPE_STR,
				default = 'Wtf is that string?'
			},
			{
				name = 'my-key-bool',
				type = CONFIG_TYPE_BOOL,
				comment = 'No way it\'s a boolean!',
				default = true
			}
		}
	}

	mycfg:save(not mycfg:load())
	print('Config string:', mycfg:get('my-key-str'))
	print('Config bool:', mycfg:get('my-key-bool'))
	print('Config int:', mycfg:get('my-key-int'))
end

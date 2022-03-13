allowHotReload(true)
if not ffi then
	ffi = require('ffi')
	ffi.cdef[[
		typedef struct GeoIPRecordTag {
			char *country_code;
			char *country_code3;
			char *country_name;
			char *region;
			char *city;

			// Остальное соедрижмое структуры нас не сильно интересует
		} GeoIPRecord;

		void *GeoIP_open(const char *path, int flags);
		GeoIPRecord *GeoIP_record_by_addr(void *gi, const char *);
		void GeoIPRecord_delete(void *gir);
		void GeoIP_delete(void *gi);
	]]

	lib = ffi.load('libGeoIP.so.1')
end

function onStart()
	local dir = io.datafolder()
	local gfile = ('%s/GeoIPCity.dat'):format(dir)
	io.ensure(dir)

	command.add('geoip', 'Detects player\'s country', CMDF_OP, function(caller, args)
		if not args then
			return 'Usage: /geoip <player name>'
		end

		local name = args:match('([a-zA-Z0-9_]+)')
		if not name then
			return '&cInvalid user name'
		end

		local client = client.getbyname(name)
		if not client then
			return '&cPlayer not found'
		end

		if client:islocal() then
			return '&cGeoIP on local clients is pointless'
		end

		if gi == nil then
			gi = lib.GeoIP_open(gfile, 2)
			if gi == nil then
				return '&cFailed to open GeoIP database'
			end
		end

		local gir = lib.GeoIP_record_by_addr(gi, client:getaddr())
		if gir == nil then
			return '&cRecord not found in GeoIP database'
		end

		local country = 'Unknown'
		if gir.country_name ~= nil then
			country = ffi.string(gir.country_name)
		end

		local city = 'Unknown'
		if gir.city ~= nil then
			city = ffi.string(gir.city)
		end

		lib.GeoIPRecord_delete(gir)

		return ('%s&f lives in &2%s&f/&a%s'):format(client:getdispname(), country, city)
	end)
end

function preReload()
	command.remove('geoip')
end

function onStop()
	command.remove('geoip')
	if gi ~= nil then
		lib.GeoIP_delete(gi)
		gi = nil
	end
end

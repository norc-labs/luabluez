-- test.lua for luahci

lhci = require("luahci")

adapter = lhci.getadapter()		-- getsomeadapter
-- why get "route" when this just returns an int dev id?
-- it actually returns a "resource number".

io.write("lua adapter: " .. adapter .. "\n")

sock = lhci.openadapter(adapter)

io.write("lua socket: " .. sock .. "\n")

r = lhci.set_scan_params(sock)
-- sock = lhci.set_scan_params(adapter, ACTIVESCAN, 0x0010, 0x0010, PUBLICADDR, FILTERNONE, 10000)

io.write("lua scan params result (sock): " .. r .. "\n")

r = lhci.lescan(sock, 0x01)	-- enable

io.write("lua lescan res: " .. r .. "\n")

-- r = lhci.lescan(sock, 0x00)	-- disable

-- io.write("lua lescan res: " .. r .. "\n")

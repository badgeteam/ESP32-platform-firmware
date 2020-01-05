import uos, gc

try:
	uos.mkdir('/lib')
except:
	pass
try:
	uos.mkdir('/apps')
except:
	pass
try:
	uos.mkdir('/cache')
except:
	pass
try:
	uos.mkdir('/config')
except:
	pass

gc.collect()

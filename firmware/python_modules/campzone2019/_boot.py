import uos, gc

try:
	uos.mkdir('/lib')
except:
	pass

gc.collect()

print("The 'appglue' API has been deprecated. Please use the 'system' API instead!")

def start_app(app, display = True):
	import system
	system.start(app, display)

def home():
	import system
	system.home(True)

import time
copy = [
	"WIFI_SSID",
	"WIFI_PASSWORD"
	]

#----

with open ("sdkconfig", "r") as sdkconfig:
    configs=sdkconfig.readlines()

constants = []

for i in configs:
	for j in copy:
		if i.startswith("CONFIG_"+j):
			constants.append(i.replace("CONFIG_","",1))

data = ''.join(constants)

with open ("python_modules/shared/_consts.py", "w") as output:
	output.write(data)

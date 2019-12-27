import sys

if len(sys.argv) < 2:
	print("Usage: {} <files>".format(sys.argv[0]))

print("names = [")
for i in range(len(sys.argv)-1):
	fname = sys.argv[1+i]
	print("\""+fname[:-4]+"\"",end=", ")
print("]")

print("data = [")
for i in range(len(sys.argv)-1):
	fname = sys.argv[1+i]
	with open(fname, "rb") as f:
		print(f.read(), end=",\n")
print("]")

# MPR121 settings for the Disobey 2020 badge
import identification

# Get machine readable identifier of the badge type
_badgeType = identification.getType()

# Parameters which are the same for every badge variant
buttons = [4,5,7,6,1,3,0,2,-1] #A, B, START, SELECT, DOWN, RIGHT, UP, LEFT, FLASH (-1 for not available)
pinToName = ["UP", "DOWN", "LEFT", "RIGHT", "A", "B", "SELECT", "START", "LED1", "LED2", "LED_PWR", "DISPLAY_PWR"]
correction_factor = 2

# Calibration reference values per badge variant
if _badgeType == b"t": # Techie
	verification = [65,51,57,56,45,63,64,72,0,0,0,0]
elif _badgeType == b"f": # Fixer
	verification = [61,54,60,54,52,60,58,60,0,0,0,0]
elif _badgeType == b"c": # Corporate
	verification = [67,47,58,48,50,58,65,76,0,0,0,0]
elif _badgeType == b"n": # Netrunner
	#verification = [61,62,54,0,0,57,70,78,0,0,0,0]
	pass # Verification for netrunner is disabled because of missing data for channels 3 and 4
elif _badgeType == b"r": # Rocker
	verification = [61,53,64,50,45,64,67,73,0,0,0,0]

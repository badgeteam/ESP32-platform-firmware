# MPR121 settings for the Disobey 2020 badge

import identification
_badgeType = identification.getType()

if _badgeType == 0: # Techie
	buttons = [4,5,7,6,1,3,0,2,-1] #A, B, START, SELECT, DOWN, RIGHT, UP, LEFT, FLASH (-1 for not available)
	pinToName = ["UP", "DOWN", "LEFT", "RIGHT", "A", "B", "SELECT", "START", "LED1", "LED2", "LED_PWR", "DISPLAY_PWR"]
	correction_factor = 2
	#verification = [65,49,56,54,48,60,65,74,0,0,0,0]
elif _badgeType == 1: # Fixer
	buttons = [4,5,7,6,1,3,0,2,-1] #A, B, START, SELECT, DOWN, RIGHT, UP, LEFT, FLASH (-1 for not available)
	pinToName = ["UP", "DOWN", "LEFT", "RIGHT", "A", "B", "SELECT", "START", "LED1", "LED2", "LED_PWR", "DISPLAY_PWR"]
	correction_factor = 2
	verification = [65,49,56,54,48,60,65,74,0,0,0,0]
elif _badgeType == 2: # Corporate
	buttons = [4,5,7,6,1,3,0,2,-1] #A, B, START, SELECT, DOWN, RIGHT, UP, LEFT, FLASH (-1 for not available)
	pinToName = ["UP", "DOWN", "LEFT", "RIGHT", "A", "B", "SELECT", "START", "LED1", "LED2", "LED_PWR", "DISPLAY_PWR"]
	correction_factor = 2
	#verification = [65,49,56,54,48,60,65,74,0,0,0,0]
elif _badgeType == 3: # Netrunner
	buttons = [4,5,7,6,1,3,0,2,-1] #A, B, START, SELECT, DOWN, RIGHT, UP, LEFT, FLASH (-1 for not available)
	pinToName = ["UP", "DOWN", "LEFT", "RIGHT", "A", "B", "SELECT", "START", "LED1", "LED2", "LED_PWR", "DISPLAY_PWR"]
	correction_factor = 2
	#verification = [65,49,56,54,48,60,65,74,0,0,0,0]
elif _badgeType == 4: # Rocker
	buttons = [4,5,7,6,1,3,0,2,-1] #A, B, START, SELECT, DOWN, RIGHT, UP, LEFT, FLASH (-1 for not available)
	pinToName = ["UP", "DOWN", "LEFT", "RIGHT", "A", "B", "SELECT", "START", "LED1", "LED2", "LED_PWR", "DISPLAY_PWR"]
	correction_factor = 2
	#verification = [65,49,56,54,48,60,65,74,0,0,0,0]
else: # Unknown / prototype
	buttons = [4,5,7,6,1,3,0,2,-1] #A, B, START, SELECT, DOWN, RIGHT, UP, LEFT, FLASH (-1 for not available)
	pinToName = ["UP", "DOWN", "LEFT", "RIGHT", "A", "B", "SELECT", "START", "LED1", "LED2", "LED_PWR", "DISPLAY_PWR"]
	correction_factor = 2

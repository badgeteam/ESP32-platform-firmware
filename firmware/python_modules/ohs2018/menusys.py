import gxgde0213b1
import G_FreeSans24pt7b
import font12
import font16
import font20
import font24
import network
import ubinascii
import urandom
import machine
import time
import os
import imagedata
from machine import Pin, TouchPad
from ohsbadge import epd
from ohsbadge import fb

class Menu:
	menuitems = []
	cur_opt = None
	menutitle = ""

	def __init__(self,title):
		self.menutitle= title
	
	# Add an item to the menu
	def addItem(self,text,function):
		i = {'text':text,'function':function}
		if self.cur_opt == None:
			self.cur_opt = i
		self.menuitems.append(i)

	def handleKey(self,key):
		f = self.cur_opt['function']
		if key == "up":
			self.move(-1)
		elif key == "down":
			self.move(1)
		elif key == "right":
			self.move(8)
		elif key == "left":
			self.move(-8)
		elif key == "launch":
			if f != None:
				print("Launching %s"%f)
				epd.init()
				epd.clear_frame(fb)
				epd.display_string_at(fb, 0, 0, "Launching", font16, gxgde0213b1.COLORED)
				epd.display_string_at(fb, 0, 16, self.cur_opt['text']+" ..", font16, gxgde0213b1.COLORED)
				epd.display_frame(fb)
				time.sleep(1)
				f(self.cur_opt['text'])
				epd.clear_frame(fb)
				epd.display_string_at(fb, 0, 0, "App Finished!", font16, gxgde0213b1.COLORED)
				epd.display_string_at(fb, 0, 16, self.cur_opt['text']+" ..", font16, gxgde0213b1.COLORED)
				epd.display_frame(fb)
				time.sleep(1)
				epd.clear_frame(fb)
				epd.display_frame(fb)
				epd.initPart()

			else:
				print("Could not launch '%s' no function attatched!"%self.cur_opt['text'])

	def move(self,num):
		m = self
		try:
			m.cur_opt = m.menuitems[m.menuitems.index(m.cur_opt)+num]
		except IndexError:
			print("invalid menu index")
	
	def drawMenu(self):
		epd.clear_frame(fb)
		ypos = 20   
		xpos = 15
		ydelta = 12
		xdelta = 125
		epd.display_string_at(fb, 0, 0, self.menutitle, font16, gxgde0213b1.COLORED)		

		
		for i in self.menuitems:
			epd.display_string_at(fb, xpos, ypos, i['text'], font12, gxgde0213b1.COLORED)
			if i == self.cur_opt:
				epd.display_string_at(fb, xpos-14, ypos, "->", font12, gxgde0213b1.COLORED)
				
			ypos +=ydelta
			if ypos>(9*12):
				ypos = 20
				xpos += xdelta
		epd.display_frame(fb)

	def menuloop(self,up,down,left,right,run,exit):
		m = self
		epd.clear_frame(fb)
		epd.display_frame(fb)
		epd.initPart()

		m.drawMenu()
		touchdelay = 0.01
		touchthres = 800
		while True:
			if up.read()<touchthres:
				m.handleKey("up")
				while up.read()<touchthres:
					time.sleep(touchdelay)
				m.drawMenu()
			if down.read()<touchthres:
				m.handleKey("down")
				while down.read()<touchthres:
					time.sleep(touchdelay)
				m.drawMenu()
			if left.read()<touchthres:
				m.handleKey("left")
				while left.read()<touchthres:
					time.sleep(touchdelay)
				m.drawMenu()
			if right.read()<touchthres:
				m.handleKey("right")
				while right.read()<touchthres:
					time.sleep(touchdelay)
				m.drawMenu()
			if run.read()<touchthres:
				m.handleKey("launch")
				while run.read()<touchthres:
					time.sleep(touchdelay)
				m.drawMenu()
			if exit.read()<touchthres:
				m.handleKey("right")
				while exit.read()<touchthres:
					time.sleep(touchdelay)
				return
		
		
			
		

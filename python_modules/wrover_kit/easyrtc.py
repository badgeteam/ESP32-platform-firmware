#For backwards compatibility

import rtc

def string(print_date=False, print_time=True, timestamp = -1):
	rtc.string(print_date, print_time, timestamp)

def configure():
	rtc.configure()

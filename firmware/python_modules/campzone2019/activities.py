import rgb, wifi, uos as os, ujson, utime, uinterface, rtc, urequests, machine, sys, gc, system

rgb.clear()

has_switched = False

## Sleep during scrolling of text (quickhack)
def waitfor(text):
    time = 1.6+ (len(text)*0.3)
    utime.sleep(time)

## Debug on display
def show_progress(text):
    rgb.clear()
    rgb.scrolltext(text,(255,255,255),(0,0))

## Callback for menu (left & right)
def call_left(pressed):
    global has_switched
    global daysindex
    global days
    if pressed:
        if(daysindex == 0):
            daysindex = len(days) -1
        else:
            daysindex -= 1
        has_switched = True

def call_right(pressed):
    global has_switched
    global daysindex
    global days
    if pressed:
        if(daysindex == (len(days)-1)):
            daysindex = 0
        else:
            daysindex += 1
        has_switched = True

## Part of update_cache 
def get_last_updated():
    last_updated = -1
    return machine.nvs_getint('activities', 'lastUpdate') or last_updated

## Part of update_cache
def set_last_updated():
    try:
        return machine.nvs_setint('activities', 'lastUpdate', int(utime.time()))
    except:
        pass

## Update cached json for activities
def update_cache():
    last_update = get_last_updated()
    if last_update > 0 and utime.time() < last_update + (600):
        global activities
        try:
            filestream = open(cache_path + cache_file,'r')
            activities = sorted(ujson.load(filestream), key= lambda x: int(x['time']))
            filestream.close()
            parse_activities()
        except:
            print("Err reading")
        return True

    print('Updating activities cache..')
    if not wifi.status():
        show_progress("Connecting to WiFi...")
        uinterface.connect_wifi()
        if not wifi.wait():
            show_progress("Failed to connect to WiFi.")
            return False

    show_progress("Downloading activities...")
    try:
        os.mkdir(cache_path)
    except:
        pass
    try:
        request = urequests.get(url, timeout=30)
        show_progress("Saving activities...")
        with open(cache_path + cache_file, 'w') as activities_file:
            activities_file.write(request.text)
        request.close()

        show_progress("Parsing activities...")
        global activities
        filestream = open(cache_path + cache_file,'r')
        activities = sorted(ujson.load(filestream), key= lambda x: int(x['time']))
        filestream.close()
        parse_activities()
        set_last_updated()
        show_progress("Done!")
        gc.collect()
        return True
    except BaseException as e:
        sys.print_exception(e)
        show_progress("Failed!")
        gc.collect()
    return False

## Parse json to activities
def parse_activities():
    prevday = 0
    events = []
    global days
    days = []
    month = ['Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct', 'Nov', 'Dec']
    try:
        for item in activities:
            day_number = utime.localtime( int(item['time']) )[7]
            if (day_number != prevday) and (prevday != 0):
                #events.append(day)
                #events
                print("New day found: "+display_date)
                day = { 'events': events[:], 'date': display_date, 'doy': prevday }
                days.append(day)
                events = []

            time_utc = int(item['time'])
            display_date = str(utime.localtime(time_utc)[2]) + ' ' + month[  utime.localtime( int(item['time']))[1] -1 ]
            prevday = day_number
            event = { 'name': item['name'], 'time': item['time'], 'type': item['type'], 'location': item['location'] }
            events.append(event)
        day = { 'events': events[:], 'date': display_date, 'doy': prevday }
        days.append(day)  
    except:
        days = [ { 'events': [ { 'name':'Undef', 'time':'0','type':'1','Location':'epoch'} ], 'date':'01 Jan', 'doy': 1 } ]
        print("Error parsing activities")


##### Main program ######

days = []

## Check if RTC is initialized
if not rtc.isSet():
    if not wifi.status():
        print("Updating time")
        uinterface.connect_wifi()
    wifi.ntp()
    wifi.disconnect()

## Cache updates
cache_path = '/cache/activities'
cache_file = '/activities.json'
url = "http://cz19.lanergy.eu/api/cz_app.php"

## Update cache
update_cache()

## Get current day of year
today = utime.localtime()[7]

counter = 0
daysindex = 0
for day in days:
    if day['doy'] == today:
       daysindex = counter
    counter += 1 

eventtype = ['Compo','Activity']

while True:
    menu = []
    rgb.clear()
    rgb.text(days[daysindex]['date'],(0,255,255),(6,1))
    rgb.setfont( rgb.FONT_6x3 )
    utime.sleep(0.5)
    for event in days[daysindex]['events']:
        time_utc = int( event['time'] )
        time_local = time_utc + 60 * 60
        hour = str(utime.localtime( time_local )[3])
        minute = str(utime.localtime( time_local )[4])
        if len(minute) != 2:
            minute = '0'+minute

        menu.append(hour + ':'+minute+ ' ' + event['name'])
    eventindex = uinterface.menu(menu,0,call_left,call_right)
    print (eventindex)
    if eventindex != None:
        event = days[daysindex]['events'][eventindex]
        eventtext = 'Type: ' + eventtype[int(event['type'])]
        eventtext += ' - Location: ' + event['location']

        rgb.clear()
        rgb.scrolltext(eventtext,(255,0,0),(0,0))
        waitfor(eventtext)
    elif has_switched:
        has_switched = False
    else:
        system.reboot()


#!/bin/bash

# Ugly little Bash script, generates a set of .h files for GFX using
# GNU FreeFont sources.  There are three fonts: 'Mono' (Courier-like),
# 'Sans' (Helvetica-like) and 'Serif' (Times-like); four styles: regular,
# bold, oblique or italic, and bold+oblique or bold+italic; and four
# sizes: 9, 12, 18 and 24 point.  No real error checking or anything,
# this just powers through all the combinations, calling the fontconvert
# utility and redirecting the output to a .h file for each combo.

# Adafruit_GFX repository does not include the source outline fonts
# (huge zipfile, different license) but they're easily acquired:
# http://savannah.gnu.org/projects/freefont/

cd input

convert=../fontconvert
header=../fontheader
inpath=
outpath=../../font_
fonts=(freesans org_01 fairlight pixelade dejavusans permanentmarker roboto)
styles=("" mono bold italic black italic blackitalic)
sizes=(6 8 9 12 13 16 18 20 22 24 36 42)

for f in ${fonts[*]}
do
	for index in ${!styles[*]}
	do
		st=${styles[$index]}
		for si in ${sizes[*]}
		do
			infile=$inpath$f$st".ttf"
			#echo $infile
			if [ -f $infile ] # Does source combination exist?
			  then
				$convert $infile $si 112 > $outpath$f$st$si"pt7b.c"
			fi
		done
	done
done

$convert weather.ttf 42 112 59905 59923 -59905 > ../../font_weather42pt8b.c

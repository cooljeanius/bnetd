#!/usr/bin/env python
# -*- Mode: Python; tab-width: 4 -*-
#
# Copyright (C) 2001-2002 Gianluigi Tiesi <sherpya@netfarm.it>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 2, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# Many thanx to <JEBs@shbe.net> for testing, ranking and unsingned exp patch
#
# ==========================================================================
__version__ = "0.95"


### Only if CGI_MODE = 1
CGI_MODE=0
FILE="/opt/bnetd/var/ladders/ladder.D2DV"
MAX=100
NUMSEP=','

from struct import unpack,calcsize
from string import find,split,join
from os import stat
from sys import argv,exit,stdout
from getopt import getopt


#### Templates
modes = [ 'html', 'ansi', 'ascii', 'python' ]
templates = {}
for m in modes:
    templates[m] = {}
    

### html ###

#
templates['html']['header']="""<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">
<html>
 <head>
  <title>D2 Closed Realm Ladder</title>
  <meta http-equiv="Content-Type" content="text/html; charset=utf-8">
 </head>
 <body bgcolor="#000000" text="#ffff00">
 <h2 style="color: lightgreen;" align="center">D2 Closed Realm Ladder</h2>
  <table style="border: solid lightblue; border-width: 1px;" align="center" border="0" width="80%" summary="">
"""

#
templates['html']['footer']="""  </table>
  <p style="color: lightblue;" align="center">Generated by ladder.py v %s - &copy; 2001-2002 <a style="color: lightgreen;" href="mailto:sherpya@netfarm.it">Sherpya</a></p>
 </body>
</html>
""" % __version__

# %s for description of ladder type
templates['html']['summary'] = """    <tr style="color: lightblue" bgcolor="#666666"><th colspan="5">Ladder for %s</th></tr>
"""

#
templates['html']['tbheader'] = """<tr style="color: lightgreen;"><th align="center">#</th><th align="left">Charname</th><th align="right">level</th><th align="center">class</th><th align="right">exp</th></tr>
"""
# %s for charname
templates['html']['normal'] = """%s"""
templates['html']['hardcore'] = { 0 : """<span style="color: red;">%s</span>""",
                                  1 : """<span style="color: orange;">%s</span>""" }

# %s charname - %d level - %s class - %d experience
templates['html']['entry'] = """<tr bgcolor="#222222"><td align="right">%d</td><td align="left">%s</td><td align="right">%d</td><td align="center">%s</td><td align="right">%s</td></tr>
"""

#
templates['html']['prefix'] = "%s %s"
templates['html']['separator'] = """<tr><td colspan="5">&nbsp;</td></tr>
"""

#### html

#### ascii / ansi
width = 68
line = '-' * width + '\n'
s_level = ' ' * 17
s_class = ' ' * 10
s_exp = ' ' * 10
text = 'D2 Closed Ladder'
esc = '\033'
off = esc + '[0m'

colors = {
        'grey': esc + '[1;30m',
        'red': esc + '[1;31m',
        'green': esc + '[1;32m',
        'yellow': esc + '[1;33m',
        'blue': esc + '[1;34m',
        'purple': esc + '[1;35m',
        'magenta': esc + '[1;36m',
        'white': esc + '[1;37m',
        'green': esc + '[1;32m'
        }

templates['ascii']['header'] = line + (int((len(line) - len(text))/ 2)) * ' ' + text + '\n' + line
templates['ascii']['footer'] = 'generated by ladder.py (c) Sherpya [sherpya@netfarm.it]\n'
templates['ascii']['summary'] = 'Ladder for %s\n\n'
templates['ascii']['tbheader'] = '   #  charname' + s_level + 'level' + s_class + 'class' + s_exp + 'exp' + '\n\n'
templates['ascii']['normal'] = '%s'
templates['ascii']['hardcore'] = { 0 : '*%s*', 1: '(%s)' }
templates['ascii']['entry'] = '%4d %-27s %2d %16s  %14s\n'
templates['ascii']['prefix'] = "%-9s %s"
templates['ascii']['separator'] = line + '\n'

line = colors['blue'] + ( '-' * width) + off + '\n'
templates['ansi']['header'] = line + (int((len(line) - len(text) - 10)/ 2)) * ' ' + colors['green'] + text + off + '\n' + line
templates['ansi']['footer'] = colors['green'] + 'generated by ' + colors['blue'] + 'ladder.py' + colors['green'] + ' (c) Sherpya [sherpya@netfarm.it]' + off + '\n'
templates['ansi']['summary'] = colors['white'] + 'Ladder for %s' + off + '\n\n'
templates['ansi']['tbheader'] = colors['green'] + '   #  charname' + s_level + 'level' + s_class + 'class' + s_exp + 'exp' + off + '\n\n'
templates['ansi']['normal'] = colors['yellow'] + '%s'
templates['ansi']['hardcore'] = { 0 : colors['red'] + '%s', 1: colors['grey'] + '%s' } 
templates['ansi']['entry'] = colors['yellow'] + '%4d %-34s %2d %16s  %14s' + off + '\n'
templates['ansi']['prefix'] = "%-9s %s"
templates['ansi']['separator'] = line + '\n'


del text
#### ascii / ansi


     
### Some struct from d2cs/d2dbs source
#
# ladder header (4 + 4 = 8):
#   bn_int maxtype
#   bn_int checksum
LD_HEAD="<2i"
szLD_HEAD = calcsize(LD_HEAD)

#
# ladder info (4 + 2 + 1 + 1 + 16 = 24):
#   bn_int experience
#   bn_short status
#   bn_byte level
#   bn_byte class;
#   char charname[16];
LD_INFO="<Ihbb16s"
szLD_INFO = calcsize(LD_INFO)

#
# ladder index (4 + 4 + 4 = 12):
#   bn_int type
#   bn_int offset
#   bn_int number
LD_INDEX="<3i"
szLD_INDEX = calcsize(LD_INDEX)

## Status flags
S_INIT = 0x1
S_EXP  = 0x20
S_HC   = 0x04
S_DEAD = 0x08


classes = {
    0x00 : ['Amazon', 'f'],
    0x01 : ['Sorceress', 'f'],
    0x02 : ['Necromancer', 'm'],
    0x03 : ['Paladin', 'm'],
    0x04 : ['Barbarian', 'm'],
    0x05 : ['Druid', 'm'],
    0x06 : ['Assassin', 'f']
    }

desc = {
    'nor': 'Diablo II',
    'exp': 'Lord of Desctruction'
    }


diff = {
    'nor': {
        0x1: { 0 : { 'm': 'Sir', 'f': 'Dame' },
               1 : { 'm': 'Count', 'f': 'Countess' }
               },
               
        0x2: { 0 : { 'm': 'Lord', 'f': 'Lady' },
               1 : { 'm': 'Duke', 'f': 'Duchess' }
               },
               
        0x3: { 0 : { 'm': 'Baron', 'f': 'Baroness' },
               1 : { 'm': 'King', 'f': 'Queen' }
               }
        },

    'exp': {
        0x1: { 0 : { 'm': 'Slayer', 'f': 'Slayer' },
               1 : { 'm': 'Destroyer', 'f': 'Destroyer' }
               },
               
        0x2: { 0 : { 'm': 'Champion', 'f': 'Champion' },
               1 : { 'm': 'Conqueror', 'f': 'Conqueror' }
               },
               
        0x3: { 0 : { 'm': 'Patriarch', 'f': 'Matriarch' },
               1 : { 'm': 'Guardian', 'f': 'Guardian' }
               }
        }
    }

## Utils
def thousand_commas(number):
    res = ''
    num = str(number)
    numlen = len(num)
    
    ### Workaround for python version < 1.6
    ### Long numbers take L when converted into strings
    
    if num[numlen-1] == "L":
        num = num[:numlen-1]
        numlen = numlen - 1

    for i in range(numlen - 1,0,-1):
        res = num[i] + res
        if ((numlen - i + 1) % 3) == 1:
            res = NUMSEP + res
        
    res = num[0] + res
    return res
                                                                     
def remove_null(text):
    return split(text, chr(0))[0]


def get_ladder(file):
    try:
        size = stat(file)[6]
        data = open(file, "rb")
    except:
        print "Error opening %s for read" % file
        exit()
    
    maxtype, checksum = unpack(LD_HEAD, data.read(szLD_HEAD))

    size = size - szLD_HEAD

    head = []

    for i in range(maxtype):
        type, offset, number = unpack(LD_INDEX, data.read(szLD_INDEX))
        size = size - szLD_INDEX
        head.append(
        {
        'type': type,
        'offset': offset,
        'number': number
        })
        

    ladder = {}
    ladder['nor'] = []
    ladder['exp'] = []

    temp = {}
    temp['nor'] = []
    temp['exp'] = []


    while size > 0:
        try:
            experience, status, level, _class, charname = unpack(LD_INFO, data.read(szLD_INFO))
        except:
            ### Bad data
            size = size - szLD_INFO
            continue
        
        size = size - szLD_INFO

        ## Avoid null chars
        if not experience:
            continue
        
        charname = remove_null(charname)
        died = 0

        if status & S_EXP:
            _type = 'exp'
            difficulty = ((status >> 0x08) & 0x0f) / 5
        else:
            _type = 'nor'
            difficulty = ((status >> 0x08) & 0x0f) / 5

        if status & S_HC:
            hc = 1
            if status & S_DEAD:
                died = 1
        else:
            hc = 0
        
        c_class = classes[_class]

        if difficulty and diff[_type].has_key(difficulty):
            prefix = diff[_type][difficulty][hc][c_class[1]]
        else:
            prefix = None
        
        char = (experience, {
            'charname'   : charname,
            'prefix'     : prefix,
            'experience' : experience,
            'class'      : c_class[0],
            'sex'        : c_class[0],
            'level'      : level,
            'type'       : _type,
            'difficulty' : difficulty,
            'hc'         : hc,
            'died'       : died
            })
        ## Dupe char? why?
        if char not in temp[_type]:
            temp[_type].append(char)
        
    data.close()

    ## Sorting by exp
    temp['nor'].sort()
    temp['nor'].reverse()
    temp['exp'].sort()
    temp['exp'].reverse()

    for _type in temp.keys():
        for ch in temp[_type]:
            ladder[_type].append(ch[1])
    del temp

    return ladder

def generate(ladder, mode, output, max):

    output.write(templates[mode]['header'])
    
    for _type in ladder.keys():
        count = 1
        output.write(templates[mode]['summary'] % desc[_type])
        output.write(templates[mode]['tbheader'])
        
        for ch in ladder[_type]:

            if ch['hc']:
                charname = templates[mode]['hardcore'][ch['died']] % ch['charname']
            else:
                charname = templates[mode]['normal'] % ch['charname']

            if ch['prefix']:
                charname = templates[mode]['prefix'] % (ch['prefix'], charname)

            experience = thousand_commas(ch['experience'])
            output.write(templates[mode]['entry'] % (count, charname, ch['level'], ch['class'], experience))
            count = count + 1
            if count > max:
                break

        output.write(templates[mode]['separator'])

    output.write(templates[mode]['footer'])


def pickle_to(ladder, output):
    try:
        from cPickle import dump
    except:
        from pickle import dump

    try:
        out = open(output, "wb")
    except:
        print "Cannot open %s for pickle dump" % output
        exit()

    dump(ladder, out)
    out.close()

### Main

### CGI MODE
if CGI_MODE:
    print "Content-Type: text/html"
    print
    ladder = get_ladder(FILE)
    generate(ladder, 'html', stdout, MAX)
    exit()

args = argv[1:]
optlist, args = getopt(args, "hi:o:m:n:")
if len(args):
    for bad in args:
        print "%s: Unrecognized option %s" % (argv[0], bad)
    exit()

### defaults
file = None
output = None # stdout
mode = modes[0]
real_max = 1000
max = 100

def show_help():
    print
    print "ladder.py v%s - (c) 2001-2002 Sherpya <sherpya@netfarm.it>" % __version__
    print "Usage: ladder.py -i ladder_file [-o outputfile] [-m mode] [-n max ladder chars]"
    print
    print "       -i ladder_file, is the ladder file like ladder.D2DV"
    print "       -o output file, if omitted defaults to stdout"
    print "       -m mode, avaiables mode are: %s, defaults to %s" % (join(modes,', '), modes[0])
    print "       -n max_char, max char to display in each ladder, defaults to %d" % max
    print 
    print "       note: python output mode creates a python object usable by pickle module"
    print

for opt in optlist:

    # Help
    if opt[0] == '-h':
        show_help()
        exit()

    # Input file
    if opt[0] == '-i':
        file = opt[1]
        continue

    # Output file
    if opt[0] == '-o':
        output = opt[1]
        continue

    # Output mode (html, ansi, ascii, python)
    if opt[0] == '-m':
        if opt[1] in modes:
            mode = opt[1]
            continue
        else:
            print "Invalid mode %s, valid modes are %s" % (opt[1], join(modes, ', '))
            exit()

    # Max chars in ladder
    if opt[0] == '-n':
        try:
            max = int(opt[1])
        except:
            max = 0

        if (max < 2) or max > real_max:
            print "Invalid value for max char in ladder must be > 1 and < %d" % real_max
            exit()
        continue

if not file:
    show_help()
    exit()

ladder = get_ladder(file)
if mode == 'python':
    if output:
        pickle_to(ladder, output)
    else:
        print "Cannot dump python object to stdout"
    exit()

if output:
    try:
        output = open(output, "wb")
    except:
        print "Cannot open %s for writing" % output
        exit()
else:
    output = stdout
    
generate(ladder, mode, output, max)
    
    

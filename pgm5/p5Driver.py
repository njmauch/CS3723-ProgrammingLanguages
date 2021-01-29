# p5Driver.py written by Nathan Mauch
# Purpose:
#   This program reads an input file with many text lines, each containing "@."
#   most containing information that will be placed in dictionaries.  There are two
#   commands, VAR, FORMAT.  The code detects errors and displays them but
#   continues.  If the line does not start with "@.", that is a line that is going to be
#   printed based on the format provided and that is currently saved in the format dictionary.
#   If the line contains "@variable", replace that variable in the line with the matching value
#   for that key
# Input:
#   Will read from command argument the file name, p4Input.txt
#   Some sample data:
#       @. VAR @first=Mae
#       @. VAR @last=Nard
#       @. VAR @petName=Pearl
#       @. FORMAT LM=5 RM=70 JUST=LEFT
# Results:
#   For each line in file, will either print array, add item associated
#   dictionary, or send line/lines to Formatter to be 
#   printed, or will display error if bad input
# Returns:
# 0 - normal

import sys
import re
import pprint
from p4At import setVariable, setFormat
from Formatter import formatter

# Empty variable dictionary to start with
variableDict = {}
# Format dictionary with default values
formatDict = {'LM': '1', 'RM': '80', 'JUST': 'LEFT', 'BULLET': 'o', 'FLOW': 'YES'}
# Read input file name
inputFile = open(sys.argv[1], 'r')
# Pattern matches to determine VARS, FORMAT, or PRINT
varPattern = re.compile(r'[@]\.\sVAR\s(.*)')
formatPattern = re.compile(r'[@]\.\sFORMAT\s(.*)')
printPattern = re.compile(r'[@]\.\s*PRINT\s*(VARS|FORMAT)')
isScaleLine = re.compile(r'^[0-9\s?]*$')
isInfoLine = re.compile(r'^Margins')
# Read each line in the file and then loop through each line
lines = inputFile.readlines()
# An array to save lines that are meant to be formatted and any possibe
# variables that need to be subsited with its value from the variable
# dictionary
paragraph = []
for line in lines:
    # if the line is a new line call formatter function to format
    # any items in the paragraph to be printed to the output
    if line == "\n":
        formatter(paragraph, formatDict, variableDict)
        # Resets the paragraph
        paragraph = []
    # Checks if that line matches any of the patterns
    matchVar = varPattern.search(line)
    matchPrint = printPattern.search(line)
    matchFormat = formatPattern.search(line)
    scaleLine = isScaleLine.search(line)
    infoLine = isInfoLine.search(line)
    # If it matches format pattern use setFormat from p4At.py
    # to attempt to add value to the format dictionary
    if matchFormat:
        # Since the format is changing use formatter function to print the 
        # paragraph in the proper format
        if paragraph:
            formatter(paragraph, formatDict, variableDict)
            paragraph = []
        setFormat(matchFormat.group(1), formatDict)
    # If it matches variable pattern use setVariable from p4At.py
    # to attempt to add the value to the variable dictionary
    elif matchVar:
        setVariable(matchVar.group(1), variableDict)
    # If it matches the print pattern determine which dictionary
    # to print and use pretty print to print dictionary
    elif matchPrint:
        whichPrint = matchPrint.group(1)
        if whichPrint == 'VARS':
            pprint.pprint(variableDict, width=30)
        elif whichPrint == 'FORMAT':
            pprint.pprint(formatDict, width=30)
        else:
            print('*** Not a recognizable print command ***')
    # If the line is a scale line from the input, print the scale line to output
    elif scaleLine:
        print(line, end='')
    # If the line describes the margins and flow, print that line to the output
    elif infoLine:
        print(line, end='')
    # If the line was not a format update, variable update, scale line, or info line
    # append that line to the paragraph to be saved and sent to the formatter to be printed
    # when it reahes the end of the paragraph
    else:
        paragraph.append(line)
# Check if there are values in paragraph that still may need to be formatted and printed
if paragraph:
    formatter(paragraph, formatDict, variableDict)


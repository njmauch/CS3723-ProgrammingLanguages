# p4At.py Written by Nathan Mauch
# Purpose:
#   Has two functions to set values to the associated dictionary,
#   setVariable and setFormat.
#

import re

# Variable dictionary to check if variable to be added is a valid key
variableDictionary = {'first': '', 'last': '', 'petName': '', 'street': '', 'city': '', 'state': '', 'zip': '',
                      'title': '', 'mortName': '', 'mortPhone': '', 'reason': ''}
# Patterns to match the patterns sent from p4Driver.py
varAtPattern = re.compile(r'@(\w*)\s?=\s?[\"]?([0-9a-zA-Z\.\- ]*)[\"]?')
justPattern = re.compile(r'(LEFT|RIGHT|BULLET|CENTER)')


# def setVariable (atString, varDictionary)
# Purpose:
#   Determines if variable and value are able to be added to dictionary
#   and will add to the dictionary if able
# Parameters:
#   I atString      The variable to be attempted to added to dictionary
#   O varDictionary Variable dictionary that variable and value will be added to
def setVariable(atString, varDictionary):
    # Checking if variable is in valid format
    varMatch = varAtPattern.search(atString)
    if varMatch:
        # Sets match group 1 to the variable name (key)
        varName = varMatch.group(1)
        # Sets match group 2 to the value
        value = varMatch.group(2)
        # If the variable name is in the dictionary add to dictionary
        if varName in variableDictionary:
            varDictionary[varName] = value
        else:
            # If variable is not in the dictionary print error
            print('*** Variable not in dictionary ***')
    else:
        # If atString is in invalid format print error.
        print('*** Invalid @varname=value ' + atString)


# def setFormat (atString, formatDictionary)
# Purpose:
#   Determines if variable and value are able to be added to dictionary
#   and will add to the dictionary if able
# Parameters:
#   I atString         The variable to be attempted to added to dictionary
#   O formatDictionary Variable dictionary that variable and value will be added to
def setFormat(atString, formatDictionary):
    rest = atString
    # Splits the string at every space in string and puts into array
    tokenM = rest.split()
    i = 0
    # While you are still in the array
    while i < len(tokenM):
        # Split the ith element of the array to get ready to insert into
        # the dictionary
        splitToken = tokenM[i].split('=', 2)
        # Sets first value to variable name (key)
        varName = splitToken[0]
        # Sets second value to the value
        value = splitToken[1]
        # Checks if the variable (key) is in the format dictionary
        if varName in formatDictionary:
            # If the variable is 'JUST' check to see the if the value
            # is ones of the allowed values
            if varName == 'JUST':
                justMatch = justPattern.search(value)
                if not justMatch:
                    # If the value for 'JUST' is bad print error
                    print('*** Bad value for JUST=, found: ' + value)
                    break
            # Add variable and value to the format dictionary
            formatDictionary[varName] = value
        else:
            # If format is bad print error
            print('*** Invalid format, found: ' + atString)
        # Go to the next value and variable to be added from the line
        i += 1

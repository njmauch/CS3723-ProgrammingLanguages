# Formatter.py written by Nathan Mauch
# Purpose:
#    Will accept a paragraph array of lines, the format dictionary, and
#    variable dictionary from p5Driver.py.  Will iterate through each line
#    in the paragraph formatting each line as necessary based on the format
#    dictionary. Left margin will be how many spaces to indent the paragraph,
#    right margin will be how many spaces will be left on the right, flow has two
#    options, yes mean is it will fill the line with as many words as possible without 
#    cutting any words off in one line, no means it will print the line if it fits
#    within the margins and then go to the next line without trying to fit more words on
#    the line even if they may fit and will also truncate any words if the extra letters 
#    do not fit in the line.  Just will determine what type of justifaction the output will
#    have either left, right, or bullet.  If it is bullet it will use the bullet value from
#    the dictionary and print that bullet.
import re
# Used to find variables in lines that will need to be substited
varAtPattern = re.compile(r'@(\w*)([\.\,]?)')


def formatter(paragraph, formatDict, variableDict):
    # Determines that justifcation of the paragraph
    # If the justification is now bullet set the number of characters
    # able to fit in the line to Right Margin - Left Margin + 1
    if formatDict["JUST"] != "BULLET":
        formatWidth = int(formatDict["RM"]) - int(formatDict["LM"]) + 1
        # Add the indent to the current line that will be printed 
        currentLine = " " * (int(formatDict["LM"]) - 1)
    # If the justification is "BULLET" set the number of characters that
    # line can hold to Right Margin - Left Margin - 1
    else:
        formatWidth = int(formatDict["RM"]) - int(formatDict["LM"]) - 1
        currentLine = " " * (int(formatDict["LM"]) - 1)
        # Adds the bullet to the first line of the paragraph
        currentLine += formatDict["BULLET"] + " "
        
    # Sets length of the line to keep track when to create new line
    lineLength = 0
    # Loop through each line that is in the paragraph array
    for line in paragraph:
        # Split that line at each space creating a tokenM for each
        # split
        tokenM = line.split()
        # Loop through the tokens created to check to see if the token
        # is a variable that is going to be substited by value with its 
        # matching key
        for token in tokenM:
            # Save the index of the token that is being checked
            index = tokenM.index(token)
            # Saves the match if found in the token
            match = varAtPattern.search(token)
            # If a match is found insert the value that matches the key
            # for that variable in the dictionary
            if match:
                token = variableDict[match.group(1)]
                # If there is a "." or "," add that to the token
                if match.group(2):
                    token += match.group(2)
                # Replace the "@variable" in the tokenM array with the variable
                # value in the dictionary
                tokenM[index] = token
        # Count for help with FLOW=NO to create new line if line ends before 
        # reaching the formatWidth
        i = 0
        if formatDict["FLOW"] == "YES":
            for word in tokenM:
                # Checks the current length of the line and if the next word
                # can fit on the line without going past the format width
                if (lineLength + len(word)) > formatWidth:
                    # Since the word can't fit in the line print the current line
                    # and reset the current line to just spaces for the indent
                    print(currentLine.ljust(formatWidth - lineLength), " ")
                    # If the justification is not bullets just set the indent 
                    # based on the left margin
                    if formatDict["JUST"] != "BULLET":
                        currentLine = " " * (int(formatDict["LM"]) - 1)
                    # If the justification is bullets add 2 extra spaces
                    # to line up paragraph without the bullet on the line
                    else:
                        currentLine = " " * (int(formatDict["LM"]) + 1)
                    # Reset the line length
                    lineLength = 0
                # Adds the word to the current line string
                currentLine += word + " "
                # Adds the length of the word plus 1 for a space to line length
                lineLength += len(word) + 1
        # If FLOW=NO
        else:
            for word in tokenM:
                # Checks the current length of the line and if the next word
                # can fit on the line without going past the format width
                if lineLength + len(word) > formatWidth:
                    # If the word does not fit on the line without exceeding
                    # the format width, truncate the word to as many characters
                    # that will fit on the line within the format width
                    currentLine += word[0:(formatWidth - lineLength)]
                    # Since the word can't fit in the line print the current line
                    # and reset the current line to just spaces for the indent
                    print(currentLine.ljust(formatWidth - lineLength), " ")
                    # If the justification is not bullets just set the indent 
                    # based on the left margin
                    if formatDict["JUST"] != "BULLET":
                        currentLine = " " * (int(formatDict["LM"]) - 1)
                    # If the justification is bullets add 2 extra spaces
                    # to line up paragraph without the bullet on the line
                    else:
                        currentLine = " " * (int(formatDict["LM"]) + 2)
                    # Reset the line length
                    lineLength = 0
                    break
                # Adds the word to the current line string
                currentLine += word + " "
                # Adds the length of the word plus 1 for a space to line length
                lineLength += len(word) + 1
                # Checks if that line is done and will create a new line rather
                # than filling the line with more words from the next line
                # even if they would fit in the format width
                if i == (len(tokenM) - 1):
                    print(currentLine.ljust(formatWidth - lineLength), " ")
                    currentLine = " " * (int(formatDict["LM"]) - 1)
                    lineLength = 0
                    break
                # Keeps track if the full line has printed
                i += 1

    print(currentLine.ljust(formatWidth - lineLength), " ")

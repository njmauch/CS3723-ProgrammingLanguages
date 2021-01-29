# p1 must use the g++ compiler because of the hashApi.cpp
# The unordered_map capabilities need the c++11 standard
CFLAGS = g++ -std=c++11 -g
INC = cs3723p1.h
OBJ = cs3723p1Driver.o cs3723p1.o hashApi.o printNode.o

# Automatic substitution using suffix rules to
# make .o files from either .c or .cpp files
%.o: %.c $(INC)
	${CFLAGS} -c $< 

%.o: %.cpp
	${CFLAGS} -c $<

# Build p1 based on the required .o files
p1: ${OBJ}
	${CFLAGS} -o p1 ${OBJ}

clean:
	rm ${OBJ}

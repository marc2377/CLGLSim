SHELL=/bin/bash
CC=g++
C_FLAGS=-g -c -Wall -O0
SRC=CLGLParser.cpp
OBJ=$(SRC:.cpp=.o)

CL_PATH_ATI=/opt/AMDAPP/include/
CL_PATH_NVIDIA=/usr/include

INCLUDE_PATH=-I../include -I../../window/include -I../../sim/include

all: $(OBJ)

%.o: %.cpp
	if [ -d "$(CL_PATH_ATI)" ]; then \
		$(CC) $(C_FLAGS) $(INCLUDE_PATH) -I$(CL_PATH_ATI) $< -o $@; \
	else \
		$(CC) $(C_FLAGS) $(INCLUDE_PATH) -I$(CL_PATH_NVIDIA) $< -o $@; \
	fi

clean:
	rm *.o

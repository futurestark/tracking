CC = g++
PROJECT = tracking_proj
SRC = Tracking.cpp
LIBS = `pkg-config --cflags --libs opencv4`
$(PROJECT) : $(SRC)
    $(CC) $(SRC) -o $(PROJECT) $(LIBS)












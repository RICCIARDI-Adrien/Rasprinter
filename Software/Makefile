CC = gcc
CCFLAGS = -W -Wall

BINARY = Rasprinter
LIBRARIES = -lSDL2 -lSDL2_ttf -pthread

all:
	$(CC) $(CCFLAGS) Main.c $(LIBRARIES) -o $(BINARY)

clean:
	rm -f $(BINARY)

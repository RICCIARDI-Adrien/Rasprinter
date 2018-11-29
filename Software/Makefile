CC = gcc
CCFLAGS = -W -Wall

BINARY = rasprinter
LIBRARIES = -lSDL2 -lSDL2_ttf

all:
	$(CC) $(CCFLAGS) Main.c $(LIBRARIES) -o $(BINARY)

clean:
	rm -f $(BINARY)

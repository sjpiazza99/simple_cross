CC=clang++
CFLAGS=-std=c++17

FILES = main.cpp simple_cross.cpp

main: $(FILES)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm main

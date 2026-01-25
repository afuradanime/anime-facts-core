CC = D:\\mingw64\\bin\\gcc.exe
CFLAGS = -Wall -O2
CFLAGS_64 = -Wall -O2
TARGET = anime_facts.exe
TARGET_LIB = anime_facts.dll

all: main build

main:
	$(CC) $(CFLAGS) -c main.c -o main.o

# Build everything
build: main
	$(CC) -o $(TARGET) main.o sqlite3/sqlite3.o

build_lib: main
	$(CC) -shared -o $(TARGET_LIB) -Wl,--out-implib,libtstdll.a main.o sqlite3/sqlite3.o

run: build
	./$(TARGET)

# Compile sqlite3
sqlite:
	$(CC) $(CFLAGS) $(SQLITE_CFLAGS) -c sqlite3/sqlite3.c -o sqlite3/sqlite3.o

clean:
	del /Q main.o 2>nul || true
	del /Q $(TARGET) $(TARGET_LIB) libtstdll.a 2>nul || true

.PHONY: sqlite main link build build_lib run clean
CC = gcc
CFLAGS = -Wall -O2
CFLAGS_64 = -Wall -O2
TARGET = anime_facts.exe
TARGET_LIB = anime_facts.dll
IMPORT_LIB = libanime_facts.a

# Source files
SRC_DIR = src
SRCS = main.c $(SRC_DIR)/dynamic_array.c $(SRC_DIR)/anime.c

# Object files
OBJS = main.o $(SRC_DIR)/dynamic_array.o $(SRC_DIR)/anime.o sqlite3/sqlite3.o

all: build

main.o: main.c include/anime.h include/anime_facts_api.h include/dynamic_array.h
	$(CC) $(CFLAGS) -c main.c -o main.o

$(SRC_DIR)/dynamic_array.o: $(SRC_DIR)/dynamic_array.c include/dynamic_array.h include/anime.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/dynamic_array.c -o $(SRC_DIR)/dynamic_array.o

$(SRC_DIR)/anime.o: $(SRC_DIR)/anime.c include/anime.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/anime.c -o $(SRC_DIR)/anime.o

# Build executable
build: $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Build DLL
build_lib: $(OBJS)
	$(CC) -shared -o $(TARGET_LIB) -Wl,--out-implib,$(IMPORT_LIB) $(OBJS)

patch: build_lib
	copy /Y anime_facts.dll ..\backend\drivers\anime_facts.dll

run: build
	./$(TARGET)

# Compile sqlite3
sqlite:
	$(CC) $(CFLAGS) $(SQLITE_CFLAGS) -c sqlite3/sqlite3.c -o sqlite3/sqlite3.o

clean:
	del /Q main.o 2>nul || true
	del /Q $(SRC_DIR)\\dynamic_array.o 2>nul || true
	del /Q $(SRC_DIR)\\anime.o 2>nul || true
	del /Q $(TARGET) $(TARGET_LIB) $(IMPORT_LIB) 2>nul || true
	@echo Cleaned build artifacts

.PHONY: sqlite main link build build_lib run clean
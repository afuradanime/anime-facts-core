# CC="D:\mingw64\bin\gcc.exe"
CC = gcc

# Detect OS and set appropriate flags and targets
ifeq ($(OS),Windows_NT)
    # Windows build
    SQLITE_CFLAGS = -DSQLITE_OS_WIN=1
    TARGET = anime_facts.exe
    TARGET_LIB = anime_facts.dll
    IMPORT_LIB = libanime_facts.a
    CFLAGS = -Wall -O2 -fPIC -DBUILDING_DLL
    CFLAGS_QUIET = -Wall -O2 -DQUIET -fPIC -DBUILDING_DLL
    BUILD_CMD = $(CC) -shared -o $(TARGET_LIB) -Wl,--out-implib,$(IMPORT_LIB) $(OBJS)
else
    # Linux/Unix build
    TARGET = anime_facts
    TARGET_LIB = libanime_facts.so
    IMPORT_LIB = 
    CFLAGS = -Wall -O2 -fPIC
    CFLAGS_QUIET = -Wall -O2 -DQUIET -fPIC
    BUILD_CMD = $(CC) -shared -o $(TARGET_LIB) $(OBJS)
endif

# Source files
SRC_DIR = src
SRCS = main.c $(SRC_DIR)/dynamic_array.c $(SRC_DIR)/anime.c $(SRC_DIR)/printer.c

# Object files
OBJS = main.o $(SRC_DIR)/dynamic_array.o $(SRC_DIR)/anime.o $(SRC_DIR)/printer.o sqlite3/sqlite3.o

BENCHMARK_EXE = benchmark.exe

all: build_lib

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

$(SRC_DIR)/dynamic_array.o: $(SRC_DIR)/dynamic_array.c include/dynamic_array.h include/anime.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/dynamic_array.c -o $(SRC_DIR)/dynamic_array.o

$(SRC_DIR)/anime.o: $(SRC_DIR)/anime.c include/anime.h
	$(CC) $(CFLAGS) -c $(SRC_DIR)/anime.c -o $(SRC_DIR)/anime.o

$(SRC_DIR)/printer.o: $(SRC_DIR)/printer.c
	$(CC) $(CFLAGS) -c $(SRC_DIR)/printer.c -o $(SRC_DIR)/printer.o

sqlite3/sqlite3.o: sqlite3/sqlite3.c sqlite3/sqlite3.h
	$(CC) $(CFLAGS) $(SQLITE_CFLAGS) -c sqlite3/sqlite3.c -o sqlite3/sqlite3.o

# Build DLL
build_lib: $(OBJS)
	$(BUILD_CMD)

build_lib_quiet: clean $(OBJS)
	$(BUILD_CMD)

patch: build_lib
	@if [ -f $(TARGET_LIB) ]; then cp $(TARGET_LIB) ../backend/drivers/$(TARGET_LIB); echo "Copied $(TARGET_LIB) to ../backend/drivers/"; else echo "Error: $(TARGET_LIB) not found"; fi

patch_test: build_lib
	@if [ -f $(TARGET_LIB) ]; then cp $(TARGET_LIB) ../test/$(TARGET_LIB); echo "Copied $(TARGET_LIB) to ../test/"; else echo "Error: $(TARGET_LIB) not found"; fi

# Becnhmarking
benchmark/benchmark.o: benchmark/benchmark.c include/anime.h include/anime_facts_api.h include/dynamic_array.h
	$(CC) $(CFLAGS) -c benchmark/benchmark.c -o benchmark/benchmark.o

benchmark: build_lib_quiet benchmark/benchmark.o
	$(CC) $(CFLAGS) -o $(BENCHMARK_EXE) benchmark/benchmark.o -L. -lanime_facts

run_benchmark: benchmark
	./$(BENCHMARK_EXE)

run: build
	./$(TARGET)

# Compile sqlite3
sqlite:
	$(CC) $(CFLAGS) $(SQLITE_CFLAGS) -c sqlite3/sqlite3.c -o sqlite3/sqlite3.o

clean:
	-rm -f main.o $(SRC_DIR)/dynamic_array.o $(SRC_DIR)/anime.o $(SRC_DIR)/printer.o sqlite3/sqlite3.o benchmark/benchmark.o $(TARGET) $(TARGET_LIB) $(IMPORT_LIB) $(BENCHMARK_EXE) 2>/dev/null || del /Q main.o $(SRC_DIR)\dynamic_array.o $(SRC_DIR)\anime.o $(SRC_DIR)\printer.o sqlite3\sqlite3.o benchmark\benchmark.o $(TARGET) $(TARGET_LIB) $(IMPORT_LIB) $(BENCHMARK_EXE) 2>nul
	@echo Cleaned build artifacts

.PHONY: sqlite main link build build_lib run clean
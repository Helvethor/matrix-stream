CC=gcc
NAME=matrix_stream

APP_NAME=matrix-stream
APP_C=src/main.c
APP_LD_FLAGS=-lrgbmatrix

LIB_C=src/$(NAME).c
LIB_H=src/$(NAME).h
LD_FLAGS=-lstdc++  -lm -pthread
C_FLAGS=

all: bin lib test-client

bin: bin/$(NAME)

bin/$(NAME): $(APP_C) $(LIB_C) $(LIB_H)
	@mkdir -p bin
	$(CC) $(APP_C) $(LIB_C) $(C_FLAGS) $(APP_LD_FLAGS) $(LD_FLAGS) -o bin/$(APP_NAME)

bin-linked: $(APP_C) lib
	@mkdir -p bin
	$(CC) $(APP_C) $(C_FLAGS) $(LD_FLAGS) -l$(NAME) -L./lib -o bin/$(APP_NAME)

test-client: src/test_client.c $(LIB_C) $(LIB_H)
	@mkdir -p bin
	$(CC) src/test_client.c $(LIB_C) $(C_FLAGS) $(LD_FLAGS) -o bin/test-client

lib: lib/lib$(NAME).so

lib/lib$(NAME).so: $(LIB_C) $(LIB_H)
	@mkdir -p lib
	$(CC) $(LIB_C) $(C_FLAGS) $(LD_FLAGS) -fPIC -shared -Wl,-soname,lib$(NAME).so -o $@

clean:
	@rm -rf bin lib

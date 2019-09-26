DIR_INC = .
DIR_SRC = .
DIR_OBJ = .
DIR_BIN = .

SRC = $(wildcard ${DIR_SRC}/*.c)  
OBJ = $(patsubst %.c,${DIR_OBJ}/%.o,$(notdir ${SRC})) 

TARGET = libinterproc.so

BIN_TARGET = ${DIR_BIN}/${TARGET}

CC = gcc 
CFLAGS = -g -Wall -I${DIR_INC}

${BIN_TARGET}:${OBJ}
	$(CC) -shared -o $@ $(OBJ) -lm -lrt -luuid `pkg-config --libs glib-2.0`
	
${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	$(CC) $(CFLAGS) -c  $< -lm -lrt -o $@ `pkg-config --cflags glib-2.0`

.PHONY:clean
clean:
	rm  ${DIR_OBJ}/*.o 
	rm  ${DIR_OBJ}/$(TARGET)

INC_PATH = 

#birary search path

# dependent library

GCC =  gcc
BIN = raw_sender
FLAGS = -rdynamic  -gstabs+
CCOPT = $(FLAGS)
CPPFLAGS = -Wall $(FLAGS) $(INC_PATH)

# all source files
all_src = ../src/raw_packet_sender.c

#all object files
OBJ = $(all_src:.c=.o)

# build excutable bin file
$(BIN) : $(OBJ)
	$(CC) $(CCOPT) -o $@ $(OBJ) $(LIB) $(LIB_PATH)

# generate object file
$(OBJ) : %.o : %.c
	$(CC) -c $(CPPFLAGS) $< -o $@ 

# clear for rebuild
clean :
	@rm -fr $(OBJ) $(BIN)

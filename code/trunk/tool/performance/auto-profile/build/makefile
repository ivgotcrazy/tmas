
#-----------------------------------
# test target define
#-----------------------------------

# target path
T_DIR_PREFIX = ../../../../src
T_PUB_DIR    = $(T_DIR_PREFIX)/pub
T_FRAME_DIR  = $(T_DIR_PREFIX)/frame
T_PROT_DIR   = $(T_DIR_PREFIX)/protocol
T_ETH_DIR    = $(T_PROT_DIR)/l2/eth
T_IPV4_DIR	 = $(T_PROT_DIR)/l3/ipv4
T_TCP_DIR	 = $(T_PROT_DIR)/l4/tcp
T_UDP_DIR    = $(T_PROT_DIR)/l4/udp
T_HTTP_DIR   = $(T_PROT_DIR)/l7/http

T_BUILD_DIR  = $(T_DIR_PREFIX)/../build

# target object files
TARGET_OBJS = $(T_FRAME_DIR)/*.o \
              $(T_PUB_DIR)/*.o \
			  $(T_TCP_DIR)/*.o \
              $(T_HTTP_DIR)/*.o

# target include path
TARGET_INC = -I$(T_FRAME_DIR)/inc \
		     -I$(T_PUB_DIR)/inc \
		     -I$(T_ETH_DIR)/inc \
		     -I$(T_IPV4_DIR)/inc \
             -I$(T_TCP_DIR)/inc \
             -I$(T_UDP_DIR)/inc \
             -I$(T_HTTP_DIR)/inc

#----------------------------------
# performance test define
#----------------------------------

# test source file directory
PROF_DIR = ../src
PROF_INC = -I$(PROF_DIR)/inc

# build para
BIN 	 = tmas-prof
CC 		 = g++
FLAGS 	 = -rdynamic --std=c++0x -gdwarf-2 -DNDEBUG -O2
CCOPT 	 = $(FLAGS)
CPPFLAGS = -Wall $(FLAGS) $(TARGET_INC) $(PROF_INC)
LIB_PATH = -L/usr/lib64/mysql

# library dependancy
LIB = -lboost_thread \
      -lboost_system \
      -lboost_program_options \
      -lboost_filesystem \
      -lboost_regex \
      -lglog \
      -lprotobuf \
      -lprotoc \
      -lpcap \
      -lmysqlclient \
	  -lgtest \
	  -lprofiler

# all source file
PROF_SRC = $(PROF_DIR)/main.cpp \
		   $(PROF_DIR)/pkt_capture_simulator.cpp 

# test object files
PROF_OBJ = $(PROF_SRC:.cpp=.o)

# build executable
$(BIN) : $(PROF_OBJ)
	$(CC) $(CCOPT) -o $@ $(PROF_OBJ) $(TARGET_OBJS) $(LIB) $(LIB_PATH)

# generate objects
$(PROF_OBJ) : %.o : %.cpp
	$(CC) -c $(CPPFLAGS) $< -o $@

# clear for rebuild
clean :
	@rm -fr $(PROF_OBJ) $(BIN)

# target
tar :
	cd $(T_BUILD_DIR); make -j9

tarn :
	cd $(T_BUILD_DIR); make ndebug=yes -j9

tar- :
	cd $(T_BUILD_DIR); make clean; make -j9

tarn- :
	cd $(T_BUILD_DIR); make clean; make ndebug=yes -j9

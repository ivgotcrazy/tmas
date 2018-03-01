
#-----------------------------------
# test target define
#-----------------------------------

# target path
T_DIR_PREFIX = ../../src
T_PUB_DIR    = $(T_DIR_PREFIX)/pub
T_FRAME_DIR  = $(T_DIR_PREFIX)/frame
T_PROT_DIR   = $(T_DIR_PREFIX)/protocol
T_L2_DIR     = $(T_PROT_DIR)/l2
T_L3_DIR	 = $(T_PROT_DIR)/l3
T_L4_DIR	 = $(T_PROT_DIR)/l4
T_TCP_DIR	 = $(T_L4_DIR)/tcp
T_UDP_DIR    = $(T_L4_DIR)/udp
T_L7_DIR	 = $(T_PROT_DIR)/l7
T_HTTP_DIR   = $(T_L7_DIR)/http

T_BUILD_DIR  = $(T_DIR_PREFIX)/../build

# target object files
TARGET_OBJS = $(T_FRAME_DIR)/*.o \
              $(T_PUB_DIR)/*.o \
              $(T_L2_DIR)/*.o \
              $(T_L3_DIR)/*.o \
              $(T_L4_DIR)/*.o \
			  $(T_TCP_DIR)/*.o \
              $(T_UDP_DIR)/*.o \
              $(T_L7_DIR)/*.o \
              $(T_HTTP_DIR)/*.o

# target include path
TARGET_INC = -I$(T_FRAME_DIR)/inc \
		     -I$(T_PUB_DIR)/inc \
		     -I$(T_L2_DIR)/inc \
		     -I$(T_L3_DIR)/inc \
		     -I$(T_L4_DIR)/inc \
             -I$(T_TCP_DIR)/inc \
             -I$(T_UDP_DIR)/inc \
		     -I$(T_L7_DIR)/inc \
             -I$(T_HTTP_DIR)/inc

#----------------------------------
# test define
#----------------------------------

# test source file directory
DIR_PREFIX = ../src
MAIN_DIR   = $(DIR_PREFIX)
PUB_DIR    = $(DIR_PREFIX)/pub
FRAME_DIR  = $(DIR_PREFIX)/frame
PROT_DIR   = $(DIR_PREFIX)/protocol
UTIL_DIR   = $(DIR_PREFIX)/util
L2_DIR     = $(PROT_DIR)/l2
L3_DIR     = $(PROT_DIR)/l3
L4_DIR     = $(PROT_DIR)/l4
TCP_DIR    = $(L4_DIR)/tcp
UDP_DIR    = $(L4_DIR)/udp
L7_DIR     = $(PROT_DIR)/l7
HTTP_DIR   = $(L7_DIR)/http

# test include file directory
TEST_INC = -I$(PUB_DIR)/inc \
		   -I$(FRAME_DIR)/inc \
		   -I$(UTIL_DIR)/inc \
		   -I$(L2_DIR)/inc \
		   -I$(L3_DIR)/inc \
		   -I$(L4_DIR)/inc \
		   -I$(TCP_DIR)/inc \
		   -I$(UDP_DIR)/inc \
		   -I$(L7_DIR)/inc \
		   -I$(HTTP_DIR)/inc

# build para
BIN = ut_tmas
CC = g++
FLAGS = -rdynamic --std=c++0x -gdwarf-2
CCOPT = $(FLAGS) -fprofile-arcs -ftest-coverage
CPPFLAGS = -Wall $(FLAGS) $(TARGET_INC) $(TEST_INC)

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
	  -lgtest

# source file
MAIN_SRC 	= $(MAIN_DIR)/main.cpp
PUB_SRC 	= $(PUB_DIR)/http_parser_test.cpp \
			  $(PUB_DIR)/pkt_parser_test.cpp
FRAME_SRC 	=
UTIL_SRC 	= $(UTIL_DIR)/eth_layer.cpp \
              $(UTIL_DIR)/pkt_construct_test.cpp 
L2_SRC 		=
L3_SRC 		= $(L3_DIR)/ip_monitor_test.cpp
L4_SRC 		=
TCP_SRC 	= $(TCP_DIR)/tcp_conn_reorder_test.cpp 
UDP_SRC 	=
L7_SRC 		=
HTTP_SRC 	= $(HTTP_DIR)/uri_config_parser_test.cpp \
		      $(HTTP_DIR)/http_monitor_test.cpp \
			  $(HTTP_DIR)/cl_http_recombinder_test.cpp \
			  $(HTTP_DIR)/chunked_http_recombinder_test.cpp

# all source file
TEST_SRC = $(MAIN_SRC) \
           $(PUB_SRC) \
           $(FRAME_SRC) \
           $(UTIL_SRC) \
           $(L2_SRC) \
           $(L3_SRC) \
           $(L4_SRC) \
           $(TCP_SRC) \
           $(UDP_SRC) \
           $(L7_SRC) \
           $(HTTP_SRC)

# test object files
TEST_OBJ = $(TEST_SRC:.cpp=.o)

# build executable
$(BIN) : $(TEST_OBJ)
	$(CC) $(CCOPT) -o $@ $(TEST_OBJ) $(TARGET_OBJS) $(LIB) $(LIB_PATH)

# generate objects
$(TEST_OBJ) : %.o : %.cpp
	$(CC) -c $(CPPFLAGS) $< -o $@

# clear for rebuild
clean :
	@rm -fr $(TEST_OBJ) $(BIN)

# target
tar :
	cd $(T_BUILD_DIR); make -j9

tar- :
	cd $(T_BUILD_DIR); make clean; make -j9

# coverage
cov :
	cd $(T_BUILD_DIR); make clean; make coverage=yes -j9
 

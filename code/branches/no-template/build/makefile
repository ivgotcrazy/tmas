# path define
prefix 	  = ../src
main_dir  = $(prefix)
pub_dir   = $(prefix)/pub
frame_dir = $(prefix)/frame
prot_dir  = $(prefix)/protocol
l2_dir 	  = $(prot_dir)/l2
l3_dir 	  = $(prot_dir)/l3
l4_dir 	  = $(prot_dir)/l4
tcp_dir   = $(l4_dir)/tcp
udp_dir   = $(l4_dir)/udp
l7_dir 	  = $(prot_dir)/l7
http_dir  = $(l7_dir)/http

# header file search path
INC_PATH = -I../.. \
	       -I$(pub_dir)/inc \
	       -I$(frame_dir)/inc \
	       -I$(l2_dir)/inc \
	       -I$(l3_dir)/inc \
	       -I$(l4_dir)/inc \
	       -I$(tcp_dir)/inc \
	       -I$(udp_dir)/inc \
	       -I$(l7_dir)/inc \
           -I$(http_dir)/inc \

# library search path
LIB_PATH = -L/usr/lib64/mysql

# dependent library
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
	  -lprofiler

# build para
CC =ccache g++
BIN = tmas
FLAGS = -rdynamic --std=c++0x -gdwarf-2

ifeq ($(ndebug), yes)
FLAGS += -DNDEBUG
endif

ifeq ($(coverage), yes)
FLAGS += -fprofile-arcs -ftest-coverage
endif

CCOPT = $(FLAGS)
CPPFLAGS = -Wall $(FLAGS) $(INC_PATH)

# main source files
main_src = $(main_dir)/main.cpp

# pub source files 
pub_src = $(pub_dir)/pkt_processor.cpp \
          $(pub_dir)/tmas_util.cpp \
		  $(pub_dir)/basic_config_parser.cpp \
		  $(pub_dir)/tmas_config_parser.cpp \
		  $(pub_dir)/mem_buf_pool.cpp \
		  $(pub_dir)/tmas_assert.cpp \
		  $(pub_dir)/timer.cpp \
		  $(pub_dir)/http_parser.cpp \
		  $(pub_dir)/pkt_parser.cpp

# frame source files
frame_src = $(frame_dir)/database_recorder.cpp \
			$(frame_dir)/pkt_capturer.cpp \
			$(frame_dir)/pkt_dispatcher.cpp
		 
# l2 source files
l2_src = $(l2_dir)/eth_monitor.cpp 

# l3 source files
l3_src = $(l3_dir)/ip_monitor.cpp

# l4 source files
l4_src = $(l4_dir)/l4_monitor.cpp \
         $(tcp_dir)/tcp_monitor.cpp \
		 $(tcp_dir)/tcp_conn_reorder.cpp \
		 $(tcp_dir)/tcp_conn_analyzer.cpp \
		 $(tcp_dir)/tcp_fsm.cpp \
		 $(udp_dir)/udp_monitor.cpp

# l7 source files
l7_src = $(l7_dir)/l7_monitor.cpp \
 		 $(http_dir)/http_monitor.cpp \
		 $(http_dir)/http_config_parser.cpp \
		 $(http_dir)/http_run_session.cpp \
		 $(http_dir)/http_recombinder.cpp \
		 $(http_dir)/http_recombinder_impl.cpp

# all source files
all_src = $(main_src) \
          $(pub_src) \
          $(frame_src) \
          $(l2_src) \
          $(l3_src) \
          $(l4_src) \
          $(l7_src)

# all object files
OBJ = $(all_src:.cpp=.o)

# build excutable bin file
$(BIN) : $(OBJ)
	$(CC) $(CCOPT) -o $@ $(OBJ) $(LIB) $(LIB_PATH)

# generate object file
$(OBJ) : %.o : %.cpp 
	$(CC) -c $(CPPFLAGS) $< -o $@

# clear for rebuild
clean :
	@rm -fr $(OBJ) $(BIN)

# path define
prefix 	  = ../src
main_dir  = $(prefix)
pub_dir   = $(prefix)/pub
frame_dir = $(prefix)/frame
prot_dir  = $(prefix)/protocol
eth_dir   = $(prot_dir)/l2/eth
ipv4_dir  = $(prot_dir)/l3/ipv4
tcp_dir   = $(prot_dir)/l4/tcp
udp_dir   = $(prot_dir)/l4/udp
http_dir  = $(prot_dir)/l7/http

# header file search path
INC_PATH = -I../.. \
	       -I$(pub_dir)/inc \
	       -I$(frame_dir)/inc \
	       -I$(eth_dir)/inc \
	       -I$(ipv4_dir)/inc \
	       -I$(tcp_dir)/inc \
	       -I$(udp_dir)/inc \
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
	  -lprofiler \
	  -lrt

# build para
CC = g++
BIN = tmas
FLAGS = -rdynamic --std=c++0x -O2 -gdwarf-2

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
pub_src = $(pub_dir)/tmas_util.cpp \
		  $(pub_dir)/basic_config_parser.cpp \
		  $(pub_dir)/tmas_config_parser.cpp \
		  $(pub_dir)/mem_buf_pool.cpp \
		  $(pub_dir)/tmas_assert.cpp \
		  $(pub_dir)/timer.cpp \
		  $(pub_dir)/http_parser.cpp \
		  $(pub_dir)/pkt_parser.cpp \
		  $(pub_dir)/ip_region_parser.cpp \
		  $(pub_dir)/port_region_parser.cpp \
		  $(pub_dir)/config_parser.cpp

# frame source files
frame_src = $(frame_dir)/pkt_capturer.cpp \
			$(frame_dir)/pkt_dispatcher.cpp \
			$(frame_dir)/pkt_distributor.cpp \
			$(frame_dir)/pkt_receiver.cpp \
			$(frame_dir)/cpu_core_manager.cpp \
			$(frame_dir)/file_logger.cpp \
			$(frame_dir)/database_logger.cpp \
			$(frame_dir)/pcap_pkt_capturer.cpp
		 
# l2 source files
l2_src = 

# l3 source files
l3_src = $(ipv4_dir)/ipv4_frag_reassembler.cpp 

# l4 source files
l4_src = $(tcp_dir)/tcp_fsm.cpp \
		 $(tcp_dir)/tcp_filter.cpp \
		 $(tcp_dir)/tcp_observer_manager.cpp \
		 $(tcp_dir)/tcp_recorder.cpp \
		 $(tcp_dir)/tcp_config_parser.cpp

# l7 source files
l7_src = $(http_dir)/http_config_parser.cpp \
		 $(http_dir)/http_run_session.cpp \
		 $(http_dir)/http_recombinder.cpp \
		 $(http_dir)/http_recombinder_impl.cpp \
		 $(http_dir)/http_recorder.cpp \
		 $(http_dir)/http_filter.cpp \
		 $(http_dir)/http_observer_manager.cpp

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

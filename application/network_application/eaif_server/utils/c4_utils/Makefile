CXX=g++

SRCS=c4_model_parser.cc
OBJS=$(SRCS:.cc=.o)
BIN=c4_model_parser

Q=@

CXXFLAGS = -g -std=c++11 -lstdc++

all: $(OBJS)
	@printf "    %-8s %s\n" "CXX" $(BIN)
	$(Q)$(CXX) -o $(BIN) $< $(CXXFLAGS)

clean:
	$(Q)rm -rf $(BIN) $(OBJS)

%.o : %.cc
	@printf "    %-8s $@\n" "CXX"
	$(Q)$(CXX) -c $< -o $@ $(CXXFLAGS)




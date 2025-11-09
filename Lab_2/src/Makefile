CXX = g++
CXXFLAGS = -std=c++11 -Wall -O2 -I.
BITSTREAM_DIR = bit_stream/src

# Object files
OBJS = GolombUtils.o $(BITSTREAM_DIR)/bit_stream.o $(BITSTREAM_DIR)/byte_stream.o

# Targets
all: golomb test_golomb

# Main program with command-line interface
golomb: golomb_main.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

golomb_main.o: golomb_main.cpp GolombUtils.h
	$(CXX) $(CXXFLAGS) -c golomb_main.cpp

# Comprehensive test program
test_golomb: test_golomb.o $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

test_golomb.o: test_golomb.cpp GolombUtils.h
	$(CXX) $(CXXFLAGS) -c test_golomb.cpp

# Common object files
GolombUtils.o: GolombUtils.cpp GolombUtils.h
	$(CXX) $(CXXFLAGS) -c GolombUtils.cpp

$(BITSTREAM_DIR)/bit_stream.o: $(BITSTREAM_DIR)/bit_stream.cpp $(BITSTREAM_DIR)/bit_stream.h
	$(CXX) $(CXXFLAGS) -c $(BITSTREAM_DIR)/bit_stream.cpp -o $@

$(BITSTREAM_DIR)/byte_stream.o: $(BITSTREAM_DIR)/byte_stream.cpp $(BITSTREAM_DIR)/byte_stream.h
	$(CXX) $(CXXFLAGS) -c $(BITSTREAM_DIR)/byte_stream.cpp -o $@

clean:
	rm -f *.o golomb test_golomb temp_golomb_test.bin *.bin $(BITSTREAM_DIR)/*.o

.PHONY: all clean

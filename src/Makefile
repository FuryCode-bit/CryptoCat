# -----------------------------------------------------------------------------
# Rainbow Table Project Makefile (Directory Structure)
# -----------------------------------------------------------------------------

CXX = g++
CXXFLAGS = -std=c++17 -Wall -O3 -pthread -I./core
LDFLAGS = -lssl -lcrypto

# Core source files
CORE_SOURCES = core/crypto.cpp core/utils.cpp

# Default target - creates bin/ directory and builds main programs
all: bin/table bin/guess bin/test_crypto bin/create_single_chain

bin:
	mkdir -p bin

# Main programs - all executables go directly in bin/
bin/table: table_generator/table_main.cpp $(CORE_SOURCES) | bin
	$(CXX) $(CXXFLAGS) -o bin/table table_generator/table_main.cpp $(CORE_SOURCES) $(LDFLAGS)

bin/guess: guesser/guess_main.cpp $(CORE_SOURCES) | bin
	$(CXX) $(CXXFLAGS) -o bin/guess guesser/guess_main.cpp $(CORE_SOURCES) $(LDFLAGS)

bin/test_crypto: test/test_crypto_main.cpp $(CORE_SOURCES) | bin
	$(CXX) $(CXXFLAGS) -o bin/test_crypto test/test_crypto_main.cpp $(CORE_SOURCES) $(LDFLAGS)

bin/create_single_chain: test/create_single_chain.cpp $(CORE_SOURCES) | bin
	$(CXX) $(CXXFLAGS) -o bin/create_single_chain test/create_single_chain.cpp $(CORE_SOURCES) $(LDFLAGS)

# Clean rule
clean:
	rm -rf bin/

# Debug info
debug:
	@echo "Core sources: $(CORE_SOURCES)"
	@echo "Checking files exist:"
	@ls -la table_generator/table_main.cpp
	@ls -la guesser/guess_main.cpp
	@ls -la $(CORE_SOURCES)

.PHONY: all test clean debug
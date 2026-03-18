CXX = g++
CXXFLAGS = -std=c++17 -Werror -O2 \
    -I. \
    -I../headers \
    -I../utils \
    -I../

SRC_FILES = \
    pareto_based/mo_features_pareto.cpp \
    walks/adaptative_walk_pareto.cpp \
    walks/random_walk_pareto.cpp \
    walks/walks_common.cpp \
    metrics_extraction.cpp \
    normalization.cpp \
    statistical_functions.cpp \
    ../utils/features_csv.cpp \
    ../utils/solution_utils.cpp \
    ../utils/generic_algorithms.cpp \
    ../utils/io_utils.cpp

MAIN_FILE = ../main.cpp

OUTPUT = exec

$(OUTPUT): $(SRC_FILES) $(MAIN_FILE)
	@echo Starting compilation of everything...
	@$(CXX) $(CXXFLAGS) -o $(OUTPUT) $(MAIN_FILE) $(SRC_FILES)
	@echo Compilation completed.

clean:
	rm -f $(OUTPUT)

rebuild: clean $(OUTPUT)

COMPILER=g++
TARGETS=datawriter \
        event \
        gates \
        logiclevel \
        netlist \
        netlistreader \
        netlistreader_verilog \
        nets \
        simulation_data \
        simulator \
        stack \
        main
MODULES=datawriter.o \
        event.o \
        gates.o \
        logiclevel.o \
        netlist.o \
        netlistreader.o \
        netlistreader_verilog.o \
        nets.o \
        simulation_data.o \
        simulator.o \
        stack.o
FLAGS=-Wall
SRC_DIR=../../sources


all		: $(TARGETS)

datawriter	          : $(SRC_DIR)/datawriter.h $(SRC_DIR)/datawriter.cpp
	$(COMPILER) -c $(FLAGS) -o datawriter.o $(SRC_DIR)/datawriter.cpp

event	                : $(SRC_DIR)/event.h $(SRC_DIR)/event.cpp
	$(COMPILER) -c $(FLAGS) -o event.o $(SRC_DIR)/event.cpp

gates		              : $(SRC_DIR)/gates.h $(SRC_DIR)/gates.cpp
	$(COMPILER) -c $(FLAGS) -o gates.o $(SRC_DIR)/gates.cpp

logiclevel	          : $(SRC_DIR)/logiclevel.h $(SRC_DIR)/logiclevel.cpp
	$(COMPILER) -c $(FLAGS) -o logiclevel.o $(SRC_DIR)/logiclevel.cpp

netlist		            : $(SRC_DIR)/netlist.h $(SRC_DIR)/netlist.cpp
	$(COMPILER) -c $(FLAGS) -o netlist.o $(SRC_DIR)/netlist.cpp

netlistreader	        : $(SRC_DIR)/netlistreader.h $(SRC_DIR)/netlistreader.cpp
	$(COMPILER) -c $(FLAGS) -o netlistreader.o $(SRC_DIR)/netlistreader.cpp

netlistreader_verilog	: $(SRC_DIR)/netlistreader.h $(SRC_DIR)/netlistreader_verilog.cpp
	$(COMPILER) -c $(FLAGS) -o netlistreader_verilog.o $(SRC_DIR)/netlistreader_verilog.cpp

nets		              : $(SRC_DIR)/nets.h $(SRC_DIR)/nets.cpp
	$(COMPILER) -c $(FLAGS) -o nets.o $(SRC_DIR)/nets.cpp

simulation_data       : $(SRC_DIR)/simulation_data.h $(SRC_DIR)/simulation_data.cpp
	$(COMPILER) -c $(FLAGS) -o simulation_data.o $(SRC_DIR)/simulation_data.cpp

simulator	            : $(SRC_DIR)/simulator.h $(SRC_DIR)/simulator.cpp
	$(COMPILER) -c $(FLAGS) -o simulator.o $(SRC_DIR)/simulator.cpp

stack	                : $(SRC_DIR)/stack.h $(SRC_DIR)/stack.cpp
	$(COMPILER) -c $(FLAGS) -o stack.o $(SRC_DIR)/stack.cpp

main		              : datawriter.o event.o gates.o logiclevel.o netlist.o netlistreader.o netlistreader_verilog.o nets.o simulation_data.o simulator.o stack.o
	$(COMPILER) $(FLAGS) $(MODULES) $(SRC_DIR)/main.cpp -o PetriLogicSimulator

clean		:
	rm -f *.o
	rm -f PetriLogicSimulator

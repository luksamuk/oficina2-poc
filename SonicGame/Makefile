CXX       = g++ --std=c++11
MKDIR     = mkdir -p
DEL       = rm -rf
LDD       = ld -r

OUTPUT    = SonicGame

SRC_DIR   = ./src/
RES_DIR   = ./resources/
OBJ_DIR   = ./obj/
OUT_DIR   = ./bin/Debug/

CXX_OBJ   = -c
CXX_FLAGS = -g -Wall
CXX_OUT   = -o
OF2_FLAGS   = `oficina2-config --cppflags`
OF2_LIBS    = `oficina2-config --libs`

PROJ_LIBS = $(OBJ_DIR)main.o $(OBJ_DIR)GameScreen.o $(OBJ_DIR)MenuScreen.o

LIBS      = $(OF2_LIBS)
FLAGS     = $(CXX_FLAGS) $(OF2_FLAGS) -I$(SRC_DIR) $(GUILE_FLAGS) $(LUA_FLAGS)

.PHONY: clean delete dirs assets




### Main target

all: debug

release: CXX_FLAGS = -O3
release: OUT_DIR   = ./bin/Release/
release: clean game

debug: game

game: dirs $(PROJ_LIBS)
	$(CXX) $(PROJ_LIBS) $(FLAGS) $(LIBS) $(CXX_OUT) $(OUT_DIR)$(OUTPUT)




### Phony Targets

assets:
	cd $(RES_DIR);  \
	make

clean:
	$(DEL) $(OBJ_DIR)*.o

delete:
	$(DEL) $(OUT_DIR)$(OUTPUT)

dirs:
	@$(MKDIR) $(OUT_DIR) $(OBJ_DIR)




### Subtargets

$(OBJ_DIR)main.o: $(SRC_DIR)main.cpp
	$(CXX) $(FLAGS) $(CXX_OBJ) $(SRC_DIR)main.cpp $(CXX_OUT) $(OBJ_DIR)main.o

$(OBJ_DIR)GameScreen.o: $(SRC_DIR)GameScreen.cpp
	$(CXX) $(FLAGS) $(CXX_OBJ) $(SRC_DIR)GameScreen.cpp $(CXX_OUT) $(OBJ_DIR)GameScreen.o

$(OBJ_DIR)MenuScreen.o: $(SRC_DIR)MenuScreen.cpp
	$(CXX) $(FLAGS) $(CXX_OBJ) $(SRC_DIR)MenuScreen.cpp $(CXX_OUT) $(OBJ_DIR)MenuScreen.o

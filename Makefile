CXX = g++
OBJECTS = foolgo.o board/position.o utility/math.o

foolgo : $(OBJECTS)
	$(CXX) -o foolgo $(OBJECTS)

$(OBJECTS) : board/def.h
foolgo.o : utility/math.h utility/common.h game/game.h player/player.h player/human_player.h player/random_player.h\
board/full_board.h board/pos_ctrl.h board/board.h board/chain_set.h board/eye_set.h
board/position.o : board/position.h
utility/math.o : utility/math.h


.PHONY : clean
clean :
	-rm foolgo.exe $(OBJECTS)
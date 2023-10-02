.PHONY: clean run

TARGET = stack
CC = g++
CFLAGS = -Wshadow -Winit-self -Wredundant-decls -Wcast-align -Wundef -Wfloat-equal -Winline -Wunreachable-code -Wmissing-declarations -Wmissing-include-dirs -Wswitch-enum -Wswitch-default -Weffc++ -Wmain -Wextra -Wall -g -pipe -fexceptions -Wcast-qual -Wconversion -Wctor-dtor-privacy -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -Werror=vla
DEFINES = -DDEBUG -DHASH -DSNITCH
PREF_OBJ = ./obj/

SRC = $(wildcard *.cpp)
OBJ = $(patsubst %.cpp, $(PREF_OBJ)%.o, $(SRC))

all: $(TARGET)
$(TARGET) : $(OBJ)
	$(CC) $(OBJ) $(CFLAGS) $(DEFINES) -o $(TARGET)

$(PREF_OBJ)%.o : %.cpp
	$(CC) -c $< -o $@ $(CFLAGS) $(DEFINES)

#sorry
clean:
	erase D:\Ucheba\C_Progs\Stack\obj\*.o
run:
	./$(TARGET)

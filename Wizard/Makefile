TARGET = build/Runtime\ Broker.exe
SRC = Wizard.cpp
OBJ = build/Wizard.o

CXX = g++
CXXFLAGS = -D_CRT_SECURE_NO_WARNINGS -DWIN32 -D_WINDOWS -std=c++17 -m32 -Iinclude

LDFLAGS = -static -static-libgcc -static-libstdc++ -m32

LIBS = -ladvapi32 -lpthread -lws2_32

all: $(TARGET)

$(TARGET): build/Wizard.o
	$(CXX) build/Wizard.o $(LIBS) $(LDFLAGS) -o $(TARGET)
	rm -rf build/Wizard.o
	@echo "| - - - - - - - |"
	@echo "| - - Exito - - |"
	@echo "| - - - - - - - |"

build/Wizard.o: $(SRC)
	$(CXX) $(CXXFLAGS) -c $(SRC) -o build/Wizard.o

clean:
	@echo "Nada que limpiar"

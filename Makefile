COMPOPT := -I"E:/include" -I"C:/GL/include" $(default_gcc_args) -std=c++23 -L"C:/GL/libs" -L"E:/libs"
SOURCES := $(wildcard *.cpp)
HEADERS := $(wildcard *.hpp)
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
libpygame.a: $(OBJECTS)
	ar rcs libpygame.a $(OBJECTS)
cleanobj:
	del *.o
clean: cleanobj
	del *.exe
%.o: %.cpp $(HEADERS)
	g++ -c $< $(COMPOPT)
2ddemo.exe: demo/2ddemo.cpp libpygame.a
	g++ $< -o$@ $(COMPOPT) -lglfw3 -lgdi32 -lglad -lfreetype -lpygame -lstrs

3ddemo.exe: demo/3ddemo.cpp libpygame.a
	g++ $< -o$@ $(COMPOPT) -lglfw3 -lgdi32 -lglad -lfreetype -lpygame -lstrs

.PHONY: cleanobj clean

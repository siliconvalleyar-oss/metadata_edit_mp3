CXX      := g++
CXXFLAGS := -std=c++17 -fPIC -Wall -Wextra -O2
LDFLAGS  := -lQt5Core -lQt5Gui -lQt5Widgets -lQt5Multimedia
INCLUDES := -Iinclude -I/usr/include/x86_64-linux-gnu/qt5 \
            -I/usr/include/x86_64-linux-gnu/qt5/QtCore \
            -I/usr/include/x86_64-linux-gnu/qt5/QtGui \
            -I/usr/include/x86_64-linux-gnu/qt5/QtWidgets \
            -I/usr/include/x86_64-linux-gnu/qt5/QtMultimedia

TARGET   := MetadataMP3
SRC      := src
OBJ      := obj
BIN      := bin

SOURCES  := $(wildcard $(SRC)/*.cpp)
OBJECTS  := $(patsubst $(SRC)/%.cpp,$(OBJ)/%.o,$(SOURCES))
MOC_HDR  := mainwindow.h mp3tablemodel.h mp3player.h
MOC_SRC  := $(patsubst %.h,$(OBJ)/moc_%.cpp,$(MOC_HDR))
MOC_OBJ  := $(patsubst %.h,$(OBJ)/moc_%.o,$(MOC_HDR))

.SECONDARY:
.PHONY: all clean distclean run

all: $(BIN)/$(TARGET)

$(OBJ):
	mkdir -p $(OBJ)

$(BIN):
	mkdir -p $(BIN)

$(OBJ)/%.o: $(SRC)/%.cpp | $(OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MF $(@:.o=.d) -c $< -o $@

$(OBJ)/moc_%.cpp: include/%.h | $(OBJ)
	moc $(INCLUDES) $< -o $@

$(OBJ)/moc_%.o: $(OBJ)/moc_%.cpp | $(OBJ)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BIN)/$(TARGET): $(OBJECTS) $(MOC_OBJ) | $(BIN)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

run: $(BIN)/$(TARGET)
	./$(BIN)/$(TARGET)

clean:
	rm -f $(OBJ)/*.o $(OBJ)/*.d $(OBJ)/moc_*.cpp $(BIN)/$(TARGET)

distclean: clean
	rm -rf $(OBJ) $(BIN)

-include $(wildcard $(OBJ)/*.d)

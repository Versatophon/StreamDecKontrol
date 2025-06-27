CC = g++
OPTIONS = -g -m64 -std=c++17 -Wall -Wpsabi -lhidapi-hidraw -lc -lpthread -lSDL3 -lGL -lturbojpeg -lfreeimage

CFLAGS = #-L/usr/include/imgui/ -L/usr/include/imgui/backends/

OBJS_DIR = ../intermediate/objs/

SRC_DIR = ../src/
IMGUI_DIR = ../src/externals/imgui/

TARGET_DIR = ../bin/

OBJS_LIST = $(OBJS_DIR)main.o\
			$(OBJS_DIR)ManagedWindow.o\
			$(OBJS_DIR)StreamDeckWindow.o\
			$(OBJS_DIR)StreamDeckSurface.o\
			$(OBJS_DIR)StreamDeckRawDevice.o\
			$(OBJS_DIR)imgui_draw.o\
			$(OBJS_DIR)imgui_impl_sdl3.o\
			$(OBJS_DIR)imgui_impl_sdlrenderer3.o\
			$(OBJS_DIR)imgui_tables.o\
			$(OBJS_DIR)imgui_widgets.o\
			$(OBJS_DIR)imgui.o

$(OBJS_DIR)main.o: $(SRC_DIR)main.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)StreamDeckRawDevice.o: $(SRC_DIR)StreamDeckRawDevice.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)ManagedWindow.o: $(SRC_DIR)ManagedWindow.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)StreamDeckWindow.o: $(SRC_DIR)StreamDeckWindow.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)StreamDeckSurface.o: $(SRC_DIR)StreamDeckSurface.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)imgui_draw.o: $(IMGUI_DIR)imgui_draw.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)imgui_impl_sdl3.o: $(IMGUI_DIR)imgui_impl_sdl3.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)imgui_impl_sdlrenderer3.o: $(IMGUI_DIR)imgui_impl_sdlrenderer3.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)imgui_tables.o: $(IMGUI_DIR)imgui_tables.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)imgui_widgets.o: $(IMGUI_DIR)imgui_widgets.cpp
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJS_DIR)imgui.o: $(IMGUI_DIR)imgui.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# link
StreamDecKontrol: $(OBJS_LIST)
	$(CC) $^ $(OPTIONS) -o $(TARGET_DIR)$@

# clean
clean:
	rm -rf $(OBJS_LIST)

# rebuild
rebuild:
	make clean
	make StreamDecKontrol

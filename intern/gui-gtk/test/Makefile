COMPILER_FLAGS = "-Wall"
COMPILER_FLAGS += "-Wpedantic"
COMPILER_FLAGS += "-Wextra"
COMPILER_FLAGS += "-O3"
COMPILER_FLAGS += "-Wshadow"
COMPILER_FLAGS += "-Wno-unused-parameter"
LINKER_FLAGS = "-lm"
GCC = gcc

GTK3_COMPILER_FLAGS = `pkg-config --cflags gtk+-3.0`
GTK3_LIBRARIES = `pkg-config --libs gtk+-3.0`

gui-example.out: example.c
	${GCC} ${GTK3_COMPILER_FLAGS} ${COMPILER_FLAGS} -o $@ $^ ${GTK3_LIBRARIES}

gui-button.out: button.c
	${GCC} ${GTK3_COMPILER_FLAGS} ${COMPILER_FLAGS} -o $@ $^ ${GTK3_LIBRARIES}

gui-switch.out: switch.c
	${GCC} ${GTK3_COMPILER_FLAGS} ${COMPILER_FLAGS} -o $@ $^ ${GTK3_LIBRARIES}

gui-key-press.out: key_press_handler.c
	${GCC} ${GTK3_COMPILER_FLAGS} ${COMPILER_FLAGS} -o $@ $^ ${GTK3_LIBRARIES}

gui-key-press-gobject.out: key_press_handler_gobject.c
	${GCC} ${GTK3_COMPILER_FLAGS} ${COMPILER_FLAGS} -o $@ $^ ${GTK3_LIBRARIES}

gui-scribble-area.out: scribble_area.c
	${GCC} ${GTK3_COMPILER_FLAGS} ${COMPILER_FLAGS} -o $@ $^ ${GTK3_LIBRARIES}

gui-image.out: image.c
	${GCC} ${GTK3_COMPILER_FLAGS} ${COMPILER_FLAGS} -o $@ $^ ${GTK3_LIBRARIES} ${LINKER_FLAGS}

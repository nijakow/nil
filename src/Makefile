EXECUTABLE = ./tales.app

SOURCES = $(shell find . -name "*.c")
HEADERS = $(shell find . -name "*.h")

OBJECTS = $(patsubst %.c,%.o,$(SOURCES))
DEPENDS = $(patsubst %.c,%.d,$(SOURCES))

LDFLAGS = 
CFLAGS = -Wall -Wextra -Werror -Wno-unused-parameter -g


%.o: %.cc
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(EXECUTABLE) $(LDFLAGS)

all: $(EXECUTABLE)

.phony: run
run: $(EXECUTABLE)
	$(EXECUTABLE)

.phony: debug
debug: $(EXECUTABLE)
	gdb $(EXECUTABLE)

.phony: leakcheck
leakcheck: $(EXECUTABLE)
	valgrind --leak-check=full --show-leak-kinds=all $(EXECUTABLE)

.phony: clean
clean:
	- $(RM) $(EXECUTABLE) $(OBJECTS) $(DEPENDS)

.phony: re
re: clean all

-include $(DEPENDS)

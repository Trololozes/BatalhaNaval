# define the C compiler to use
CC = gcc

#define any compile-time flags
CFLAGS = -Wall -std=gnu11

# define any directories containing header files other than /usr/include
# ie.: -I../include
INCLUDES =

# define library paths in addition to /usr/lib
# ie.: -L../lib
LFLAGS =

# define any libraries to link into executable:
# ie.: -lm -lmylib
LIBS =

# define the C source files
SRC =

# define the executable file
TARGET =

# define the C object files
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#       For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRC
# with the .o suffix
OBJ = $(SRC:.c=.o)

# define the dependency files
DEP = $(SRC:.c=.d)

#
# The following part of the makefile is generic; it can be used to
# build any executable just by changing the definitions above
#
# Typing 'make' will invoke the first target entry in the file
# (in this case the all target entry)
# you can name this target entry anything, but "default" or "all"
# are the most commonly used names by convention
#
# This uses a suffix replacement rule for building from .c's
# it uses automatic variables
#   $@: The name of the target of the rule.
#   $^: The names of all the prerequisites, with spaces between them.
#   $<: The name of the first prerequisite.
# (see the gnu make manual section about automatic variables)
#

all: $(TARGET)

debug: $(TARGET)
debug: CFLAGS += -g

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LFLAGS) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -c $< -o $@

clean:
	rm $(OBJ) $(DEP) $(TARGET)

-include $(DEP)

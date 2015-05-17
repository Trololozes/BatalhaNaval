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
S_LIBS = -pthread
C_LIBS = -pthread

# define the C source files
S_SRC = server.c
C_SRC = client.c

# define the executable file
SERVER = battleship-server
CLIENT = battleship-client

# define the C object files
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#       For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .c of all words in the macro SRC
# with the .o suffix
S_OBJ = $(S_SRC:.c=.o)
C_OBJ = $(C_SRC:.c=.o)

# define the dependency files
S_DEP = $(S_SRC:.c=.d)
C_DEP = $(C_SRC:.c=.d)

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

all: $(SERVER) $(CLIENT)

debug: $(SERVER) $(CLIENT)
debug: CFLAGS += -g

$(SERVER): $(S_OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LFLAGS) $(S_LIBS)

$(CLIENT): $(C_OBJ)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LFLAGS) $(C_LIBS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -MMD -c $< -o $@

clean:
	$(RM) $(S_OBJ) $(C_OBJ) $(S_DEP) $(C_DEP) $(SERVER) $(CLIENT)

-include $(S_DEP) $(C_DEP)

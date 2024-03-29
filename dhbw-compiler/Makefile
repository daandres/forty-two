# DHBW CC directories
OBJDIR = bin
SRCDIR = src

# Executables
CC = gcc
BISON = bison
FLEX = flex
RM = rm
DOXYGEN = doxygen
SPLINT = splint
VALGRIND = valgrind

# Compiler Flags
STANDARDS = -std=c99 -D_POSIX_C_SOURCE=200809L
INCLUDES = -I$(SRCDIR) -I$(OBJDIR) -I$(SRCDIR)/include
CFLAGS = -O0 -Wall -Wextra -pedantic -g $(INCLUDES) $(STANDARDS) -Wno-unused-parameter
CFLAGS_GEN = -O2 -g -I$(OBJDIR) -w $(INCLUDES) $(STANDARDS)
BFLAGS_DEBUG = 
FFLAGS_DEBUG =

.PHONY: all check test
all: compiler 

###############################################
# DHBW Compiler                               #
###############################################
DHBWCC_OBJECTS = $(addprefix $(OBJDIR)/, scanner.o parser.o main.o diag.o resource_manager.o symtab.o ir_code.o typecheck.o)

## Convenient targets for DHBW Compiler
.PHONY: compiler cc_objects parser symtab typecheck ir_code scanner 
compiler: $(OBJDIR)/yacp
cc_objects: $(GENERATED_OBJECTS)
parser: $(OBJDIR)/parser.c $(OBJDIR)/parser.h
scanner: $(OBJDIR)/scanner.c
symtab: $(OBJDIR)/symtab.c
ir_code: $(OBJDIR)/ir_code.c
typecheck: $(OBJDIR)/typecheck.c

$(OBJDIR)/yacp: $(DHBWCC_OBJECTS) 
	$(CC) $(CFLAGS) -o $@ $+

$(OBJDIR)/diag.o: $(SRCDIR)/diag.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/resource_manager.o: $(SRCDIR)/resource_manager.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/main.o: $(SRCDIR)/main.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/scanner.o: $(OBJDIR)/scanner.c $(OBJDIR)/parser.h
	$(CC) $(CFLAGS_GEN) -c $< -o $@
	
$(OBJDIR)/symtab.o: $(SRCDIR)/symtab.c 
	$(CC) $(CFLAGS) -c $< -o $@
	
$(OBJDIR)/ir_code.o: $(SRCDIR)/ir_code.c 
	$(CC) $(CFLAGS) -c $< -o $@
	
$(OBJDIR)/typecheck.o: $(SRCDIR)/typecheck.c 
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/parser.o: $(OBJDIR)/parser.c $(OBJDIR)/parser.h
	$(CC) $(CFLAGS_GEN) -c $< -o $@

$(OBJDIR)/parser.c $(OBJDIR)/parser.h: $(SRCDIR)/parser.y
	$(BISON) $(BFLAGS_DEBUG) -d -v --locations --defines=$(OBJDIR)/parser.h \
	  -o $(OBJDIR)/parser.c -rall --report-file=$(OBJDIR)/bison.out  $<
	  #--report-file=$(OBJDIR)/bison.out 

$(OBJDIR)/scanner.c: $(SRCDIR)/scanner.l
	$(FLEX) $(FFLAGS_DEBUG) -o $(OBJDIR)/scanner.c $<
	#$(FLEX) -o $(OBJDIR)/scanner.c --bison-bridge --bison-locations $<

.PHONY: clean_compiler
clean_compiler:
	$(RM) -f bin/*


###############################################
# DHBW Compiler  Debug                        #
###############################################
.PHONY: debug
debug: BFLAGS_DEBUG = -t
debug: FFLAGS_DEBUG = -T -d
debug: all


###############################################
# DHBW Compiler RT-Check                      #
###############################################
SPLINT_OPTIONS = -unrecog
VALGRIND_OPTIONS = --leak-check=full

.PHONY: check_compiler
check_compiler:
	$(VALGRIND) $(VALGRIND_OPTIONS) bin/dhbwcc -p a.c

###############################################
# Doxygen Project Documentation               #
###############################################
DOXYGEN_CONFIG = doxygen/config
DOXYGEN_FLAGS = 

.PHONY: doxygen clean_doxygen
doxygen:
	$(DOXYGEN) $(DOXHGEN_FLAGS) $(DOXYGEN_CONFIG)

clean_doxygen:
	$(RM) -rf doxygen/html

.PHONY: clean
clean: clean_compiler clean_doxygen
	

CC = gcc
CFLAGS = -std=c99 -Wextra -Wall -Werror -pedantic
LDFLAGS =

ECHO = @
ifeq ($(VERBOSE),yes)
	ECHO=
endif

ifeq ($(DEBUG),no)
	CFLAGS += -O3 -DNDEBUG
	LDFLAGS +=
else
	CFLAGS += -g 
	LDFLAGS +=
endif

# Répertoires
SRC_DIR = source
LIB_DIR = Lib
OBJ_DIR = obj

# Fichiers
EXEC = routP
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

# Inclusion des headers
CFLAGS += -I$(LIB_DIR)

all:
ifeq ($(DEBUG),yes)
	@echo "Generating client in debug mode"
else
	@echo "Generating client in release mode"
endif
	@$(MAKE) $(EXEC)

# Création du répertoire obj si nécessaire
$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(EXEC): $(OBJ_DIR) $(OBJ)
	$(ECHO)$(CC) -o $@ $(OBJ) $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(ECHO)$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean mrproper

clean:
	$(ECHO)rm -rf $(OBJ_DIR)

mrproper: clean
	$(ECHO)rm -rf $(EXEC)

# Dépendances (à adapter selon tes fichiers)
$(OBJ_DIR)/tabrout.o: $(LIB_DIR)/tabrout.h
$(OBJ_DIR)/routP.o: $(LIB_DIR)/tabrout.h
# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: vdurand <vdurand@student.42lyon.fr>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/01/13 23:20:17 by val               #+#    #+#              #
#    Updated: 2026/02/03 15:19:44 by vdurand          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

LOG = echo

#	ANSI COLORS
BLACK = \033[30m
RED = \033[31m
GREEN = \033[32m
YELLOW = \033[33m
BLUE = \033[34m
MAGENTA = \033[35m
CYAN = \033[36m
WHITE = \033[37m

BG_BLACK = \033[40m
BG_RED = \033[41m
BG_GREEN = \033[42m
BG_YELLOW = \033[43m
BG_BLUE = \033[44m
BG_MAGENTA = \033[45m
BG_CYAN = \033[46m
BG_WHITE = \033[47m

BOLD = \033[1m
DIM = \033[2m
ITALIC = \033[3m
UNDERLINE = \033[4m
BLINK = \033[5m
REVERSE = \033[7m
HIDDEN = \033[8m
RESET = \033[0m

# Verbose control
VERBOSE = 0
ifeq ($(VERBOSE),1)
	SILENT =
	DUMP_OUT =
else
	SILENT = @
	DUMP_OUT = > /dev/null 2>&1
endif

# Project
NAME = webserv
SRC_DIR = src
OBJ_DIR = obj
INC_DIR = src
LIBS_DIR = libs

# Source files
SRC_FILES = main.cpp

SRC = $(addprefix $(SRC_DIR)/, $(SRC_FILES))
OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEP = $(patsubst $(SRC_DIR)/%.cpp, $(OBJ_DIR)/%.d, $(SRC))

# Libs
LIBS_DIRS :=
LIBS =
LIBS_NO_LIB =
LIBS_INCLUDE_DIRS :=

# Compiler & flags
CXX = c++
CXXFLAGS = -O3 -Wall -Werror -Wextra -std=c++98

INC_FLAGS = -I$(INC_DIR) $(addprefix -I,$(LIBS_DIRS)) $(addprefix -I,$(LIBS_INCLUDE_DIRS))
LDFLAGS =

all: $(NAME)

$(NAME): $(OBJ) $(LIBS)
	$(SILENT) $(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)
	@$(LOG) "$(BG_GREEN)>>> Program $(NAME) compiled!$(RESET)"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp Makefile
	$(SILENT) mkdir -p $(dir $@)
	@$(LOG) "$(BLUE)>>> Compiling $<...$(RESET)"
	$(SILENT) $(CXX) $(CXXFLAGS) -MMD -MP $(INC_FLAGS) -c $< -o $@

$(LIBS): %.a:
	$(SILENT) \
	if [ -f "$(@D)/configure" ]; then \
		$(LOG) "$(CYAN)>>> Found configure in $(notdir $(@D)), running it...$(RESET)"; \
		cd "$(@D)" && ./configure $(DUMP_OUT); \
		if [ $$? -ne 0 ]; then \
			$(LOG) "$(RED)>>> ./configure failed in $(notdir $(@D)) – aborting.$(RESET)"; \
			exit 1; \
		else \
			exit 0; \
		fi; \
	fi; \
	$(LOG) "$(MAGENTA)>>> Compiling library $(notdir $@)...$(RESET)"; \
	$(MAKE) -C $(dir $@) > /dev/null 2> make_errors.log || { \
		$(LOG) "$(RED)>>> Error compiling $(notdir $@):$(RESET)"; \
		cat make_errors.log; rm -f make_errors.log; exit 1; }; \
	rm -f make_errors.log; \
	if $(MAKE) -C $(dir $@) -n bonus $(DUMP_OUT); then \
		$(LOG) "$(DIM)$(MAGENTA)>>> Bonus rule exists, compiling...$(RESET)"; \
		$(MAKE) -C $(dir $@) bonus > /dev/null 2> make_errors.log || { \
			$(LOG) "$(RED)>>> Error compiling bonus for $(notdir $@):$(RESET)"; \
			cat make_errors.log; rm -f make_errors.log; exit 1; }; \
		rm -f make_errors.log; \
	fi; \
	$(LOG) "$(BG_BLUE)$(GREEN)>>> Compilation of $(notdir $@) completed!$(RESET)"

clean:
	@$(LOG) "$(YELLOW)>>> Cleaning objects$(RESET)"
	$(SILENT) rm -rf $(OBJ_DIR)

fcleanlibs:
	$(SILENT) for dir in $(LIBS_DIRS); do \
		$(MAKE) -C $$dir clean $(DUMP_OUT); \
		$(MAKE) -C $$dir fclean $(DUMP_OUT); \
		$(LOG) "$(GREEN)>>> Cleaned all in $$dir$(RESET)"; \
	done

fclean: clean fcleanlibs
	@$(LOG) "$(YELLOW)>>> Cleaning executable...$(RESET)"
	$(SILENT) rm -f $(NAME)

re: fclean all

# Targets
.PHONY: all clean fclean re fcleanlibs

-include $(DEP)

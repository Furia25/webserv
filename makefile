NAME = webserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
CPPFLAGS = -I includes

SRC_DIR = src
OBJ_DIR = .objs

SRC = \
	$(SRC_DIR)/main.cpp \
	$(SRC_DIR)/SimpleServer.cpp \
	$(SRC_DIR)/StartServer.cpp \
	$(SRC_DIR)/Sockets/SimpleSocket.cpp \
	$(SRC_DIR)/Sockets/BindingSocket.cpp \
	$(SRC_DIR)/Sockets/ConnectingSocket.cpp \
	$(SRC_DIR)/Sockets/ListeningSocket.cpp

OBJ = $(SRC:$(SRC_DIR)/%.cpp=$(OBJ_DIR)/%.o)
DEP = $(OBJ:.o=.d)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)

fclean: clean
	rm -f $(NAME)

re: fclean all

-include $(DEP)

.PHONY: all clean fclean re

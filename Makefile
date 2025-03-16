NAME = webserv

src = main.cpp server.cpp utils.cpp config.cpp config_utils.cpp

OBJ = $(src:.cpp=.o)

CC = c++ -std=c++98

CFLAGS = -Wall -Wextra -Werror

all: $(NAME) clean

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp server.hpp
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean
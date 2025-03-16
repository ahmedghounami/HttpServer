NAME	= webserv

src		=	main.cpp server.cpp utils.cpp \
			Parsing/parsing.cpp Parsing/parsingUtils.cpp
OBJ		=	$(src:.cpp=.o)
HEADER	=	server.hpp


CC = c++ -std=c++98
CFLAGS = -Wall -Wextra -Werror

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(NAME)

%.o: %.cpp $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean
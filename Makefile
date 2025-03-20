NAME	=	webserv

src		=	main.cpp server/server.cpp server/utils.cpp config/config.cpp config/config_utils.cpp \
			ParsingRequest/parsingHeader.cpp ParsingRequest/parsingBody.cpp ParsingRequest/parsingUtils.cpp \
			HandlingRequest/handleRequest.cpp

OBJ = $(src:.cpp=.o)

CC = c++ -std=c++98

CFLAGS = -Wall -Wextra -Werror

all: $(NAME)

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
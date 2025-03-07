NAME	=	webserv

SRCS	=	main.cpp
OBJS	=	$(SRCS:.cpp=.opp)
HEADER	=	Webserv.hpp

CC		=	c++
CFLAGS	=	-Wall -Wextra -Werror -std=c++98

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(OBJS) -o $(NAME)

%.opp: %.cpp $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

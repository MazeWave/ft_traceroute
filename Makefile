# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: ldalmass <marvin@42.fr>                    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/01/09 11:23:20 by maze              #+#    #+#              #
#    Updated: 2026/01/15 15:51:34 by ldalmass         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	=	ft_ping
NAME_B	=	ft_ping_bonus

SRC		=	src/main.c \
			src/log.c \
			src/parser.c \
			src/socket.c \
			src/icmp_packet.c \

OBJS	=	$(SRC:.c=.o)

CC		=	gcc

CFLAGS	=	-Wall -Wextra -Werror -std=c23 -fsanitize=address

RM		=	rm -rf

#rules
%.o : %.c
	@$(CC) $(CFLAGS) -c $< -o $@
all : $(NAME)

$(NAME) : $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@$(RM) $(NAME).o

bonus : all
	@ln -sf $(NAME) $(NAME_B)

clean :
	@$(RM) $(OBJS)

fclean : clean
	@$(RM) $(NAME) $(NAME_B)
	@$(RM)

re : fclean all
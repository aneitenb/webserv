# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/01/20 12:43:09 by aneitenb          #+#    #+#              #
#    Updated: 2025/03/20 15:02:38 by ivalimak         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME	=	webserv

BUILD	=	normal

CC				=	c++
cflags.common	=	-Wall -Wextra -Werror -std=c++17
cflags.debug	=	-g
cflags.fsan		=	$(cflags.debug) -fsanitize=address,undefined
cflags.normal	=	-O3
cflags.extra	=	
CFLAGS			=	$(cflags.common) $(cflags.$(BUILD)) $(cflags.extra)

SRCDIR	=	srcs
OBJDIR	=	obj
INCDIR	=	includes

FILES	=	main.cpp \
			ConfigFile.cpp \
			ConfigErrors.cpp

SRCS	=	$(addprefix $(SRCDIR)/, $(FILES))
OBJS	=	$(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

all: $(NAME)

$(NAME): $(OBJDIR) $(OBJS)
	@printf "\e[1;38;5;42mWEBSERV >\e[m Compiling %s\n" $@
	@$(CC) $(CFLAGS) -I$(INCDIR) $(OBJS) -o $(NAME)
	@printf "\e[1;38;5;42mWEBSERV >\e[m \e[1mDone!\e[m\n"

$(OBJDIR):
	@printf "\e[1;38;5;42mWEBSERV >\e[m Creating objdir\n"
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@printf "\e[1;38;5;42mWEBSERV >\e[m Compiling %s\n" $@
	@$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	@rm -f $(OBJS)

fclean: clean
	@rm -rf $(OBJDIR)
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re

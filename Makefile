# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: aneitenb <aneitenb@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/01/20 12:43:09 by aneitenb          #+#    #+#              #
#    Updated: 2025/03/31 13:50:43 by aneitenb         ###   ########.fr        #
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

CONFIGDIR	=	config
SERVERDIR	=	server

CONFIGFILES	=	ConfigErrors.cpp \
				ConfigFile.cpp \
				LocationBlock.cpp \
				ServerBlock.cpp
				

#SERVERFILES	=	EventLoop.cpp \
				#Server.cpp

FILES	=	main.cpp \
			$(addprefix $(CONFIGDIR)/, $(CONFIGFILES)) \
			#$(addprefix $(SERVERDIR)/, $(SERVERFILES))

SRCS	=	$(addprefix $(SRCDIR)/, $(FILES))
OBJS	=	$(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

all: $(NAME)

$(NAME): $(OBJDIR) $(OBJS)
	@printf "\e[1;38;5;42mWEBSERV >\e[m Compiling %s\n" $@
	@$(CC) $(CFLAGS) -I$(INCDIR) $(OBJS) -o $(NAME)
	@printf "\e[1;38;5;42mWEBSERV >\e[m \e[1mDone!\e[m\n"

$(OBJDIR):
	@printf "\e[1;38;5;42mWEBSERV >\e[m Creating objdir\n"
	@mkdir -p $(OBJDIR)/$(CONFIGDIR)
#	@mkdir -p $(OBJDIR)/$(SERVERDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@printf "\e[1;38;5;42mWEBSERV >\e[m Compiling %s\n" $@
	@$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	@rm -f $(OBJS)

fclean: clean
	@rm -rf $(OBJDIR)
	@rm -f $(NAME)

re: fclean all

db:
	@printf "\e[1;38;5;42mWEBSERV >\e[m Creating compilation command database\n"
	@compiledb make --no-print-directory BUILD=$(BUILD) cflags.extra=$(cflags.extra) | sed -E '/^##.*\.\.\.$$|^[[:space:]]*$$/d'
	@printf "\e[1;38;5;42mWEBSERV >\e[m \e[1mDone!\e[m\n"

.PHONY: all clean fclean re db

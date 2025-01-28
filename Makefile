# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: aneitenb <aneitenb@student.hive.fi>        +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2025/01/20 12:43:09 by aneitenb          #+#    #+#              #
#    Updated: 2025/01/28 10:42:09 by aneitenb         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME = webserv
FLAGS = -Wall -Wextra -Werror -std=c++98

SRC = main.cpp \
		ConfigFile.cpp

OBJ_DIR = obj/
SRC_DIR = srcs/

OBJ = $(addprefix $(OBJ_DIR), $(SRC:.cpp=.o))

all : $(NAME)

$(OBJ_DIR)%.o : $(SRC_DIR)%.cpp
	@c++ $(FLAGS) -c $< -o $@

$(OBJ_DIR):
	@mkdir -p $(OBJ_DIR)

$(NAME) : $(OBJ_DIR) $(OBJ)
	@c++ $(FLAGS) $(OBJ) -o $@
	@echo "\033[32;1mProgram webserv is Ready!\033[0m"

clean:
	@rm -rf $(OBJ_DIR)
	@echo "\033[32;1mRemoved object files\033[0m"

fclean: clean
	@rm -rf $(NAME)
	@echo "\033[32;1mProgram removed\033[0m"

re: fclean all
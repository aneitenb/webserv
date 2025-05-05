## ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
## ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
## ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
## ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
## ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
##  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
##
## <<Makefile>> -- <<Aida, Ilmari, Milica>>

NAME	=	webserv

BUILD	=	normal

CC				=	c++
cflags.common	=	-Wall -Wextra -Werror -std=c++17
cflags.debug	=	-g -D__DEBUG
cflags.fsan		=	$(cflags.debug) -fsanitize=address,undefined
cflags.normal	=	-O3
cflags.extra	=	
CFLAGS			=	$(cflags.common) $(cflags.$(BUILD)) $(cflags.extra)

SRCDIR	=	srcs
TESTDIR	=	tests
OBJDIR	=	obj
INCDIR	=	includes

HTTPDIR		=	http
CONFIGDIR	=	config
SERVERDIR	=	server

HTTPFILES	=	Request.cpp

SERVERFILES	=	Client.cpp \
				Listener.cpp \
				VirtualHost.cpp \
				WebServer.cpp 
				# EventLoop.cpp

CONFIGFILES	=	ConfigErrors.cpp \
				ConfigFile.cpp \
				LocationBlock.cpp \
				ServerBlock.cpp

FILES	=	main.cpp \
			CommonFunctions.cpp \
			$(addprefix $(HTTPDIR)/, $(HTTPFILES)) \
			$(addprefix $(CONFIGDIR)/, $(CONFIGFILES)) \
			$(addprefix $(SERVERDIR)/, $(SERVERFILES))

SRCS	=	$(addprefix $(SRCDIR)/, $(FILES))
OBJS	=	$(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

REQUEST_TEST	=	$(TESTDIR)/request_test

all: $(NAME)

tests: httptests
	@printf "\e[1;38;5;42mWEBSERV >\e[m All tests passed!\n"

httptests: $(REQUEST_TEST)
	@./run_test Request $(REQUEST_TEST)
	@printf "\e[1;38;5;42mWEBSERV >\e[m All http tests passed!\n"

$(REQUEST_TEST): $(SRCDIR)/$(HTTPDIR)/Request.cpp $(TESTDIR)/$(HTTPDIR)/Request.cpp
	@printf "\e[1;38;5;42mWEBSERV >\e[m Compiling Request test\n" $@
	@$(CC) $(CFLAGS) -I$(INCDIR) $^ -o $@

$(NAME): $(OBJDIR) $(OBJS)
	@printf "\e[1;38;5;42mWEBSERV >\e[m Compiling %s\n" $@
	@$(CC) $(CFLAGS) -I$(INCDIR) $(OBJS) -o $@
	@printf "\e[1;38;5;42mWEBSERV >\e[m \e[1mDone!\e[m\n"

$(OBJDIR):
	@printf "\e[1;38;5;42mWEBSERV >\e[m Creating objdir\n"
	@mkdir -p $(OBJDIR)/$(HTTPDIR)
	@mkdir -p $(OBJDIR)/$(CONFIGDIR)
	@mkdir -p $(OBJDIR)/$(SERVERDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@printf "\e[1;38;5;42mWEBSERV >\e[m Compiling %s\n" $@
	@$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	@rm -f $(OBJS)

tclean:
	@rm -f $(REQUEST_TEST)

fclean: clean tclean
	@rm -rf $(OBJDIR)
	@rm -f $(NAME)

re: fclean all

retest: tclean tests

db:
	@printf "\e[1;38;5;42mWEBSERV >\e[m Creating compilation command database\n"
	@compiledb make --no-print-directory BUILD=$(BUILD) cflags.extra=$(cflags.extra) | sed -E '/^##.*\.\.\.$$|^[[:space:]]*$$/d'
	@printf "\e[1;38;5;42mWEBSERV >\e[m \e[1mDone!\e[m\n"

.PHONY: all tests httptests clean tclean fclean re retest db

## ██╗    ██╗    ███████╗    ██████╗     ███████╗    ███████╗    ██████╗     ██╗   ██╗
## ██║    ██║    ██╔════╝    ██╔══██╗    ██╔════╝    ██╔════╝    ██╔══██╗    ██║   ██║
## ██║ █╗ ██║    █████╗      ██████╔╝    ███████╗    █████╗      ██████╔╝    ██║   ██║
## ██║███╗██║    ██╔══╝      ██╔══██╗    ╚════██║    ██╔══╝      ██╔══██╗    ╚██╗ ██╔╝
## ╚███╔███╔╝    ███████╗    ██████╔╝    ███████║    ███████╗    ██║  ██║     ╚████╔╝
##  ╚══╝╚══╝     ╚══════╝    ╚═════╝     ╚══════╝    ╚══════╝    ╚═╝  ╚═╝      ╚═══╝
##
## <<Makefile>> -- <<Aida, Ilmari, Milica>>

NAME	=	webserv

ifeq ($(shell git branch --show-current), main)
BUILD	=	normal
else
BUILD	=	fsan
endif

ifeq ($(BUILD), normal)
CC			=	c++
LOG_LEVEL	=	3
else
CC			=	g++
LOG_LEVEL	=	4
endif

AS	=	as
LD	=	ld

cflags.common	=	-Wall -Wextra -Werror -std=c++17 -DLOG_LEVEL=$(LOG_LEVEL) -DSERVER_NAME=\"$(NAME)\"
cflags.debug	=	-g -D__DEBUG
cflags.fsan		=	$(cflags.debug) -fsanitize=address,undefined
cflags.normal	=	-O3
cflags.extra	=	
CFLAGS			=	$(cflags.common) $(cflags.$(BUILD)) $(cflags.extra)

asflags.common	=	--fatal-warnings
asflags.debug	=	-gstabs
asflags.extra	=	
ASFLAGS			=	$(asflags.common) $(asflags.$(BUILD)) $(asflags.extra)

LDFLAGS_PQ_PRE	=	-dynamic-linker /lib/ld-linux-x86-64.so.2 /usr/lib/x86_64-linux-gnu/crt1.o /usr/lib/x86_64-linux-gnu/crti.o -lc
LDFLAGS_PQ_POST	=	/usr/lib/x86_64-linux-gnu/crtn.o

SRCDIR	=	srcs
TESTDIR	=	tests
OBJDIR	=	obj
INCDIR	=	includes

CGIDIR		=	cgi
HTTPDIR		=	http
UTILSDIR	=	utils
CONFIGDIR	=	config
SERVERDIR	=	server

HTTPFILES	=	Request.cpp \
				Response.cpp

SERVERFILES	=	Client.cpp \
				Listener.cpp \
				WebServer.cpp \
				EventLoop.cpp \
				CGIHandler.cpp

CONFIGFILES	=	ConfigErrors.cpp \
				ConfigFile.cpp \
				LocationBlock.cpp \
				ServerBlock.cpp

FILES	=	main.cpp \
			CommonFunctions.cpp \
			$(UTILSDIR)/message.cpp \
			$(UTILSDIR)/Timeout.cpp \
			$(addprefix $(HTTPDIR)/, $(HTTPFILES)) \
			$(addprefix $(CONFIGDIR)/, $(CONFIGFILES)) \
			$(addprefix $(SERVERDIR)/, $(SERVERFILES))

SRCS	=	$(addprefix $(SRCDIR)/, $(FILES))
OBJS	=	$(patsubst $(SRCDIR)/%.cpp, $(OBJDIR)/%.o, $(SRCS))

REQUEST_TEST	=	$(TESTDIR)/request_test

PRINT_QUERIES	=	webpage/cgi-bin/print_queries

PQ_SRCS	=	$(SRCDIR)/$(CGIDIR)/print_queries.s
PQ_OBJS	=	$(patsubst $(SRCDIR)/%.s, $(OBJDIR)/%.o, $(PQ_SRCS))

all: $(NAME) $(PRINT_QUERIES)

tests: httptests
	@printf "\e[1;38;5;42mWEBSERV >\e[m All tests passed!\n"

httptests: $(REQUEST_TEST)
	@./run_test Request $(REQUEST_TEST)
	@printf "\e[1;38;5;42mWEBSERV >\e[m All http tests passed!\n"

$(REQUEST_TEST): $(SRCDIR)/$(UTILSDIR)/message.cpp $(SRCDIR)/$(HTTPDIR)/Request.cpp $(SRCDIR)/$(CONFIGDIR)/ServerBlock.cpp $(SRCDIR)/$(CONFIGDIR)/LocationBlock.cpp $(SRCDIR)/$(CONFIGDIR)/ConfigErrors.cpp $(TESTDIR)/$(HTTPDIR)/Request.cpp
	@printf "\e[1;38;5;42mWEBSERV >\e[m Compiling Request test\n" $@
	@$(CC) $(CFLAGS) -I$(INCDIR) $^ -o $@

$(NAME): $(OBJDIR) $(OBJS)
	@printf "\e[1;38;5;42mWEBSERV >\e[m Compiling %s\n" $@
	@$(CC) $(CFLAGS) -I$(INCDIR) $(OBJS) -o $@
	@printf "\e[1;38;5;42mWEBSERV >\e[m \e[1mDone!\e[m\n"

$(PRINT_QUERIES): $(OBJDIR) $(PQ_OBJS)
	@printf "\e[1;38;5;42mWEBSERV >\e[m Linking %s\n" $@
	@$(LD) $(LDFLAGS_PQ_PRE) $(PQ_OBJS) $(LDFLAGS_PQ_POST) -o $@
	@printf "\e[1;38;5;42mWEBSERV >\e[m \e[1mDone!\e[m\n"

$(OBJDIR):
	@printf "\e[1;38;5;42mWEBSERV >\e[m Creating objdir\n"
	@mkdir -p $(OBJDIR)/$(CGIDIR)
	@mkdir -p $(OBJDIR)/$(HTTPDIR)
	@mkdir -p $(OBJDIR)/$(UTILSDIR)
	@mkdir -p $(OBJDIR)/$(CONFIGDIR)
	@mkdir -p $(OBJDIR)/$(SERVERDIR)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	@printf "\e[1;38;5;42mWEBSERV >\e[m Compiling %s\n" $@
	@$(CC) $(CFLAGS) -I$(INCDIR) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	@printf "\e[1;38;5;42mWEBSERV >\e[m Assembling %s\n" $@
	@$(AS) $(ASFLAGS) $< -o $@

clean:
	@rm -f $(OBJS)

tclean:
	@rm -f $(REQUEST_TEST)

fclean: clean tclean
	@rm -rf $(OBJDIR)
	@rm -f $(PRINT_QUERIES)
	@rm -f $(NAME)

re: fclean all

retest: tclean tests

db:
	@printf "\e[1;38;5;42mWEBSERV >\e[m Creating compilation command database\n"
	@compiledb make --no-print-directory BUILD=$(BUILD) cflags.extra=$(cflags.extra) | sed -E '/^##.*\.\.\.$$|^[[:space:]]*$$/d'
	@printf "\e[1;38;5;42mWEBSERV >\e[m \e[1mDone!\e[m\n"

.PHONY: all tests httptests clean tclean fclean re retest db

.WAIT:

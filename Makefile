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

LDDIR	=	/usr/lib/x86_64-linux-gnu

LDFLAGS_PQ_PRE	=	-dynamic-linker $(LDDIR)/ld-linux-x86-64.so.2 $(LDDIR)/crt1.o $(LDDIR)/crti.o -lc
LDFLAGS_PQ_POST	=	$(LDDIR)/crtn.o

SRCDIR	=	srcs
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

PRINT_QUERIES	=	webpage/cgi-bin/print_queries

PQ_SRCS	=	$(SRCDIR)/$(CGIDIR)/print_queries.s
PQ_OBJS	=	$(patsubst $(SRCDIR)/%.s, $(OBJDIR)/%.o, $(PQ_SRCS))

all: $(NAME) $(PRINT_QUERIES)

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

fclean: clean
	@rm -rf $(OBJDIR)
	@rm -f $(PRINT_QUERIES)
	@rm -f $(NAME)

re: fclean all

db:
	@printf "\e[1;38;5;42mWEBSERV >\e[m Creating compilation command database\n"
	@compiledb make --no-print-directory BUILD=$(BUILD) cflags.extra=$(cflags.extra) | sed -E '/^##.*\.\.\.$$|^[[:space:]]*$$/d'
	@printf "\e[1;38;5;42mWEBSERV >\e[m \e[1mDone!\e[m\n"

.PHONY: all clean fclean re db

.WAIT:

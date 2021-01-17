# **************************************************************************** #
#                                                                              #
#                                                         ::::::::             #
#    Makefile                                           :+:    :+:             #
#                                                      +:+                     #
#    By: sam <sam@student.codam.nl>                   +#+                      #
#                                                    +#+                       #
#    Created: 2020/10/02 15:16:50 by sam           #+#    #+#                  #
#    Updated: 2020/12/10 14:26:10 by peerdb        ########   odam.nl          #
#                                                                              #
# **************************************************************************** #

NAME = webserv
FILES = main split utils Base64 Enums Cgi \
		Server Client Location \
		Connection RequestParser ResponseHandler
SRCS = $(addprefix srcs/, $(addsuffix .cpp, $(FILES)))
OBJS = $(SRCS:.cpp=.o)
INCLUDE = -Iincludes

CXXFLAGS = -W -Wall -Werror -Wextra -pedantic -std=c++98
ifneq ($(filter 1, $(DEBUG) $(ASAN)),)
 CXXFLAGS += -g
 ifdef ASAN
  CXXFLAGS += -fsanitize=address -fno-omit-frame-pointer
 endif
else
 CXXFLAGS += -Ofast
endif

GNL = getnextline.a
LIBFT = libft.a
CGI_TESTER = cgi_tester
ifeq ($(shell uname), Linux)
 ECHO = -e
 CGI_TESTER = ubuntu_cgi_tester
endif
ifdef CONNECTION_LOGS
 CXXFLAGS += -D CONNECTION_LOGS=$(CONNECTION_LOGS)
endif
ifdef BONUS
 CXXFLAGS += -D BONUS=$(BONUS) -pthread
 FILES += Worker ThreadPool Mutex
endif

# Colours
DARK_GREEN	= \033[0;32m
GREEN		= \033[0;92m
END			= \033[0;0m

PREFIX		= $(DARK_GREEN)$(NAME) $(END)\xc2\xbb

all: $(NAME)

$(NAME): $(OBJS) $(LIBFT) $(GNL)
	@echo $(ECHO) "$(PREFIX)$(GREEN) Bundling executable... $(END)$(NAME)"
	@$(CXX) $(CXXFLAGS) $(OBJS) $(GNL) $(LIBFT) $(INCLUDE) -o $@
	mkdir -p htmlfiles/Downloads
	cp test/$(CGI_TESTER) YoupiBanane/youpi.bla

%.a: %
	@echo $(ECHO) "$(PREFIX)$(GREEN) Compiling file $(END)$< $(GREEN)to $(END)$@"
	@make -s -C $<
	@cp $</$@ .

%.o: %.cpp
	@echo $(ECHO) "$(PREFIX)$(GREEN) Compiling file $(END)$< $(GREEN)to $(END)$@"
	@$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

clean:
	@echo $(ECHO) "$(PREFIX)$(GREEN) Removing .o files $(END)$(OUT_DIR)"
	@rm -f $(OBJS) htmlfiles/Downloads/file_should_exist_after
	@rm -f srcs/*.o
	@make clean -s -C getnextline
	@make clean -s -C libft

fclean: clean
	@echo $(ECHO) "$(PREFIX)$(GREEN) Removing executable $(END)$(OUT_DIR)"
	@rm -rf webservwebserv.dSYM *.a
	@rm -rf htmlfiles/Downloads/*
	@make fclean -s -C getnextline
	@make fclean -s -C libft
	@rm -rf $(NAME)

re: fclean all

run: clean all
	./$(NAME)

thread: $(OBJS) $(LIBFT) $(GNL)
	@echo $(ECHO) "$(PREFIX)$(GREEN) Bundling executable... $(END)$(NAME)"
	@$(CXX) $(CXXFLAGS) -fsanitize=thread $(OBJS) $(GNL) $(LIBFT) $(INCLUDE) -o $(NAME)
	mkdir -p htmlfiles/Downloads
	cp test/$(CGI_TESTER) YoupiBanane/youpi.bla
	TSAN_OPTIONS=second_deadlock_stack=1 ./$(NAME)

.PHONY: clean fclean re all

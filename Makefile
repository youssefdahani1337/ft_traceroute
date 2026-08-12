CC          = cc
CFLAGS      = -Wall -Wextra -Werror #-fsanitize=address
NAME        = ft_traceroute

DIR_UTILS   = src/
DIR_OBJ     = obj/

UTILSFILES  = main.c resolve_host.c parse_arguments.c utils.c sender.c output.c

# Prefixes src/ to every file: src/main.c src/parser.c
SRCS        = $(addprefix $(DIR_UTILS), $(UTILSFILES))

# Transforms src/main.c into obj/main.o
OBJS        = $(UTILSFILES:%.c=$(DIR_OBJ)%.o)

all: $(NAME)
	@echo "\033[032m ready \033[0m"

# Link the executable
$(NAME): $(OBJS)
	@$(CC) $(CFLAGS) $(OBJS) -o $(NAME)
	@echo "\033[032m compiled successfully !!\033[0m"

# Compile .c from src/ into .o in obj/
$(DIR_OBJ)%.o: $(DIR_UTILS)%.c
	@mkdir -p $(DIR_OBJ)
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@rm -rf $(DIR_OBJ)
	@echo "\033[034m object files are cleaned !!\033[0m"

fclean: clean
	@rm -f $(NAME)
	@echo "\033[034m binary cleaned !!\033[0m"

re: fclean all

test: all
	@echo "\033[032m running tests !!\033[0m"
		sudo ./test.sh


.PHONY: all clean fclean re
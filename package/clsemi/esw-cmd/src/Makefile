CC		= $(CROSS_COMPILE)gcc
STRIP		= $(CROSS_COMPILE)strip
RTK_NAME = esw_cli
OBJ_DIR = build
RTK_SRC += cmd.o

RTK_OBJ = $(addprefix $(OBJ_DIR)/, $(RTK_SRC:.c=.o))
CFLAGS +=  -I. -I./inc/ 
#LDFLAGS += -static  

%.o:  ../%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

all: $(RTK_OBJ)
	$(CC) $^ $(LDFLAGS) -o bin/${RTK_NAME} 
	$(STRIP) bin/$(RTK_NAME)

clean:
	@rm bin/* -rf
	@rm build/* -rf

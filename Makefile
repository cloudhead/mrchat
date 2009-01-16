LDFLAGS := -lncurses
OBJS = otp.o tcp.o mrchat.o
CC = GCC

all: mrchat

mrchat: $(OBJS)
	$(CC) $(LDFLAGS) -o mrchat $(OBJS)

otp.o:
	$(CC) -c otp.c

tcp.o:
	$(CC) -c tcp.c

clean:
	rm mrchat $(OBJS)


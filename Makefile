LDFLAGS := -lncurses
OBJS = debug.o deskap.o OTP.o TCP_normal.o mrchat.o

all: mrchat keygen

mrchat: $(OBJS)
	$(CC) $(LDFLAGS) -o mrchat $(OBJS)

keygen:
	$(CC) -o keygen keygen.c

debug.o:
	$(CC) -c debug.c

deskap.o: debug.o
	$(CC) -c deskap.c

OTP.o: debug.o
	$(CC) -c OTP.c

TCP_normal.o: debug.o
	$(CC) -c TCP_normal.c

clean:
	rm mrchat keygen $(OBJS)


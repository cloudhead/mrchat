#include <getopt.h>
#include "common.h"
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <ncurses.h>
#include <arpa/inet.h>

#define TITLE   "Mr. Chat"
#define MSG_ERROR 2
#define MSG_ALERT 1
#define ID_SIZE  16
#define MSG_SIZE 256
#define BUF_SIZE 256


/* Commands */
#define CMD_PING "/ping"
#define CMD_EXIT "/exit"
#define CMD_ENC "/enc"
#define CMD_SCRL "/scroll"
#define CMD_ME   "/me"

/* Chat GUI */
#define WIN_CHAT_Y  1
#define WIN_CHAT_X  0
#define WIN_CHAT_H  LINES - WIN_MSG_H - 1
#define WIN_CHAT_W  COLS - WIN_STAT_W - 1
#define WIN_MSG_Y   LINES - WIN_MSG_H
#define WIN_MSG_X   0
#define WIN_MSG_H   3
#define WIN_MSG_W   COLS - WIN_STAT_W - 1
#define WIN_STAT_Y  1
#define WIN_STAT_X  COLS - WIN_STAT_W
#define WIN_STAT_H  LINES - WIN_MSG_H - 1
#define WIN_STAT_W  20

#define PORT 1337

struct node
{
	char* buf;
        struct node* previous;
        struct node* next;
};

/* Chat session */
struct session
{
      char *buf[ BUF_SIZE ];      /* Dialog buffer */
      char **status;              /* Status bar */
      int statCount;
      int total;                  /* Total number of chars typed */
      int free;                   /* Free pointer for buf[]*/
      int end;                    /* Scroll end pointer */
      int y, x;                   /* Current cursor position */
      int sock;			  /* Socket file descriptor */
};

/* User info */
struct user
{
      char id[ ID_SIZE ];
      char *ip;
      struct sockaddr_in sa;
      struct vector *msg;
};

struct Cipher
{
	char *name;
	void *(*init)( char* );
	char** (*prep_stat)( char **, int *, int );
	int (*update_stat)( char **, int, void *, int );
        struct vector *(*encrypt)( struct vector*, void* );
        struct vector *(*decrypt)( struct vector*, void* );
};

struct Network
{
	char *name;
	char** (*prep_stat)( char **, int *, int );
	int (*update_stat)( char **, void *, int );
	void *(*connect)( char* );
	void *(*listen)( char* );
	int (*close)( void * );
	int (*write)( void*, struct vector*, char );
	int (*read)( void*, struct vector*, unsigned int, char );
};

/* Key Structure */
/*
struct key
{
    char *key;
    unsigned long charCount[ ASCII ];
    int *charPos[ ASCII ];
    char bLength;
};
*/

int printBuf( struct session*, struct user*, WINDOW* );
int updateBuf( struct session*, struct user* );
int getSocketData( int, struct vector*, int );
void printMsg( WINDOW*, char*, int );
void printTitle( WINDOW* );
void updateStatus( struct session*, WINDOW* );
void clrMsgBox( WINDOW* );
void shift( char*, int, int );

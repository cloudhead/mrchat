/*
------------------------------------------------------------
Filename: deskap.h
By: desty
Date: April 2003
------------------------------------------------------------
*/
#include "common.h"
#include <sys/time.h>

#define DESKAP
#define ASCII 256
#define VERSION "\n\nDeskap 1.0 [April 2003]\n\n"
#define HELP_MSG  "Usage:\ndeskap [-Vdhrvw] -k key [-i inputfile] [-o outputfile]\n"
#define COMMON

#ifndef TRUE
#define TRUE  1
#define FALSE 0
#endif

#ifndef NULL
#define NULL 0
#endif

/* For stats */
enum
{
	DESKAP_FILE,
	DESKAP_CHARS_SENT,
	DESKAP_SIZE,
	DESKAP_END,
};

struct deskap_key
{
    unsigned char *key;
    char *file;
    unsigned long fileLength;
    unsigned long charCount[ ASCII ];
    int *charPos[ ASCII ];
    char bLength;
};

unsigned long int getSize( FILE* );
int loadKey( FILE*, struct deskap_key* );
struct vector* deskap_encrypt( struct vector *input, struct deskap_key *ctx );
struct vector* deskap_decrypt( struct vector *input, struct deskap_key *ctx );
struct deskap_key *deskap_init( char *keyFile );

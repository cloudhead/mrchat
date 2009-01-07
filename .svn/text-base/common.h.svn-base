#ifndef COMMON

#define COMMON
#define DEBUG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define bis(target,mask)  ((target) |=  (mask))
#define bic(target,mask)  ((target) &= ~(mask))
#define bit(target,mask)  ((target) &   (mask))

#ifndef min
# define min(a,b) (((a)<(b)) ? (a) : (b))
#endif

#ifndef max
# define max(a,b) (((a)<(b)) ? (b) : (a))
#endif

#ifndef align
# define align(a) (((ub4)a+(sizeof(void *)-1))&(~(sizeof(void *)-1)))
#endif

#ifndef abs
# define abs(a)   (((a)>0) ? (a) : -(a))
#endif

#define TRUE  1
#define FALSE 0
#define SUCCESS 0

enum
{
      STAT_USERS,
      STAT_SCROLL,
      STAT_END
};

struct vector
{
	unsigned int size;
        char *buf;
};

#ifdef DEBUG
	char* logFile;
#endif

/* Cipher Function Declairations */
char			**deskap_prep_stat( char **, int *, int );
int 			deskap_update_stat( char**, int, void *, int );
struct deskap_key	*deskap_init( char* );
struct vector		*deskap_encrypt( struct vector*, struct deskap_key* );
struct vector		*deskap_decrypt( struct vector*, struct deskap_key* );

char		**OTP_prep_stat( char **, int *, int );
int 		OTP_update_stat( char**, int, void *, int );
struct OTP_key	*OTP_init( char* );
struct vector	*OTP_encrypt( struct vector *, struct OTP_key * );
struct vector	*OTP_decrypt( struct vector *, struct OTP_key * );

/* Network Function Declairations */
char** TCP_normal_prep_stat( char **stat, int *statCount, int size );
int TCP_normal_update_stat( char** stat, void *attr, int size );
struct TCP_normal_attr *TCP_normal_connect( char *sAttr );
struct TCP_normal_attr *TCP_normal_listen( char *sAttr );
int TCP_normal_close( struct TCP_normal_attr *attr );
int TCP_normal_write( struct TCP_normal_attr *s, struct vector *input, char opts );
int TCP_normal_read( struct TCP_normal_attr *attr, struct vector *input, unsigned int maxsize, char opts );

#ifdef DEBUG
	char* logFile;
#endif

#endif




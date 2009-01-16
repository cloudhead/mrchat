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

/* otp */
char		**otp_prep_stat( char **, int *, int );
int 		otp_update_stat( char**, int, void *, int );
struct otp_key	*otp_init( char* );
struct vector	*otp_encrypt( struct vector *, struct otp_key * );
struct vector	*otp_decrypt( struct vector *, struct otp_key * );

/* Network Function Declairations */
char** tcp_prep_stat( char **stat, int *statCount, int size );
int tcp_update_stat( char** stat, void *attr, int size );
struct tcp_attr *tcp_connect( char *sAttr );
struct tcp_attr *tcp_listen( char *sAttr );
int tcp_close( struct tcp_attr *attr );
int tcp_write( struct tcp_attr *s, struct vector *input, char opts );
int tcp_read( struct tcp_attr *attr, struct vector *input, unsigned int maxsize, char opts );

#ifdef DEBUG
	char* logFile;
#endif

#endif




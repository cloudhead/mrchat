/*
---------------------------------------------------------
mrchat.c

By: desty and kapone
Date: April 2003
Part of: deskap

desty@7rc.org
kapone@7rc.org

7rc.org
----------------------------------------------------------
*/

#include "mrchat.h"
#define OPTIONS "-c:e:hi:k:l:n:sv"
#define CYPH_DESKAP 1
#define CYPH_AES 2

struct Cipher ciphers[] = {
{ "Plaintext", NULL, NULL, NULL, NULL, NULL },
{ "AES", NULL, NULL, NULL, NULL, NULL },
{ "Deskap", (void *)deskap_init, deskap_prep_stat, deskap_update_stat, (void *)deskap_encrypt, (void *)deskap_decrypt },
{ "OTP", (void *)OTP_init, OTP_prep_stat, OTP_update_stat, (void *)OTP_encrypt, (void *)OTP_decrypt },
{ 0, 0, 0, 0, 0, 0 }
};

struct Network networks[] = {
{ "TCP_normal", TCP_normal_prep_stat, TCP_normal_update_stat, (void *)TCP_normal_connect, (void *)TCP_normal_listen, TCP_normal_close, TCP_normal_write, TCP_normal_read },
{ 0, 0, 0, 0, 0 }
};


struct option long_options[] =
{
    { "connect", no_argument, NULL, 'c' },
    { "enc", required_argument, NULL, 'e' },
    { "help", no_argument, NULL, 'h' },
    { "id", required_argument, NULL, 'i' },
    { "key", required_argument, NULL, 'k' },
    { "listen", optional_argument, NULL, 'l'},
    { "network", required_argument, NULL, 'n' },
    { "stat", no_argument, NULL, 's' },   
    { "verbose", no_argument, NULL, 'v' }, 
    { 0, 0, 0, 0 }
};

struct Network	*network;
struct Cipher 	*cipher;

int main( int argc, char *argv[] )
{
    void 		*ctx = NULL;
    void		*netattr = NULL;
    struct session 	s;  	/* Chat session */
    struct user 	u;     	/* Local user */
    struct user 	v;     	/* Peer user */
    struct Cipher 	*cipher = &ciphers[ 0 ];
    struct Network	*network = &networks[ 0 ];
    
    char ans, arg;
    char *temp[2] = { NULL };
    int c, i = 1, n = 0;
    int showStat = FALSE;
    char verboseFlag = FALSE;
    int connectFlag = FALSE;
    int encryptFlag = FALSE;
    int option_index;

    WINDOW *chatWin;   /* Main window */
    WINDOW *msgWin;    /* Message box */
    WINDOW *statWin;   /* Status window */
    WINDOW *titleWin;  /* Title bar */
    
    cipher = &ciphers[ 0 ];
    network = &networks[ 0 ];
    
    /* Nullify session */
    memset( &s, '\x00', sizeof( s ) );
    bzero( &u, sizeof( struct user ) );
    bzero( &v, sizeof( struct user ) );
    u.msg = (struct vector *)calloc( sizeof( struct vector ), 1 );
    u.msg->size = MSG_SIZE;
    u.msg->buf = (char *)calloc( MSG_SIZE, 1 );
    v.msg = (struct vector *)calloc( sizeof( struct vector ), 1 );
    v.msg->size = MSG_SIZE;
    v.msg->buf = (char *)calloc( MSG_SIZE, 1 );

    printf( "MrChat version 0.8 initialized;\n" );
    
    /* Open logfile for debug information */
    #ifdef DEBUG
    	printf( "DEBUG mode on\n" );

	printf( "using logfile log_%i\n", getpid() );
	logFile = calloc( 10, 1 );
	snprintf( logFile, 10, "log_%i", getpid() );
	fclose( fopen( logFile, "w" ) );
    #endif

    while( ( arg = getopt_long( argc, argv, OPTIONS, long_options, &option_index ) ) != EOF )
    {
	switch( arg )
        {
		case 'c':
			connectFlag = 1;
			temp[ 1 ] = optarg;
			break;
                case 'e':
			for( i = 1; strcmp( ciphers[ i ].name, optarg ); )
				if( ciphers[ ++i ].name == 0 )
				{
					printf( "!Invalid Cipher! %s\n", optarg );
					exit( 0 );
				}
			cipher = &ciphers[ i ];
			break;
		case 'h': 
			printf( "MrChat:\n\t-c ip [port](default:1337)\n\t-e cipher to be used(default:%s)\n", ciphers[0].name );
			for( i = 0; ciphers[ i ].name != NULL;i++ )
				printf( "\t\t%s\n", ciphers[ i ].name );
			printf( "\t-i nick to be used\n\t-k key to use\n\t-l [port](default:1337)\n\t-n Connection type(default:%s)\n", networks[0] );
			for( i = 0; networks[ i ].name != NULL; i++ )
				printf( "\t\t%s\n", network[ i ].name );
			exit( 0 );
                case 'i': strcpy( u.id, optarg ); break;
		case 'k': temp[ 0 ] = optarg; break;
		case 'l':
			connectFlag = 0;
			if( optarg != NULL )
				temp[ 1 ] = optarg;
			break;
		case 'n':
			for( i = 1; strcmp( network[ i ].name, optarg ); )
				if( network[ ++i ].name == 0 )
				{
					printf( "!Invalid Connection Type! %s\n", optarg );
					exit( 0 );
				}
			cipher = &ciphers[ i ];
			break;               
		case 's': showStat = TRUE; break;
                case 'v': verboseFlag = TRUE; break;
                default : printf( "mrchat: invalid argument: '%c\n'", arg ); exit( 0 );
        }
    }
    
    if( cipher->init != NULL )
    	ctx = (void *)cipher->init( temp[ 0 ] );

    /* Client */
    if( connectFlag )
    {
	while( ( netattr = network->connect( temp[ 1 ] ) ) == NULL )
	{
		printf( "connect failed\n" );
		sleep( 5 );
	}

	/* Send nickname */
	u.msg->size = 16;
	memcpy( u.msg->buf, u.id, u.msg->size );
	network->write( netattr, u.msg, 1 );
	printf( "mrchat: sending id: %s\n", u.id );
	printf( "mrchat: waiting for peer to accept..." );
	network->read( netattr, v.msg, 1, 1 );
	
	/* Check if connection was accepted */
	if( v.msg->buf[ 0 ] == 'y' )
	{
	    /* Get peer nickname */
	    if( network->read( netattr, v.msg, 16, 1 ) < 0 );
	    memcpy( v.id, v.msg->buf, v.msg->size );
	    printf( "mrchat: connected to %s\n", v.id );
	}
	else if( v.msg->buf[ 0 ] == 'n' )
	{
	    printf( "mrchat: your connection request was denied\n" );
	    network->close( netattr );
	    exit( 0 );
	}
    }
    /* Server */
    else
    {
    	while( 1 )
	{
		if( !( netattr = network->listen( temp[ 1 ] ) ) )
		{
			sleep( 5 );
			continue;
		}
	
		//getSocketData( sock, v.id, 16 );
		network->read( netattr, v.msg, 16, 1 );
		memcpy( v.id, v.msg->buf, v.msg->size );
		
		u.msg->size = 1;
		do
		{
			printf( "mrchat: accept connection %s?", v.id );
			scanf( "%c", u.msg->buf );
		}
		while( u.msg->buf[ 0 ] != 'n' && u.msg->buf[ 0 ] != 'y' );
	
		network->write( netattr, u.msg, 1 );
	
		if( ans == 'n' )
		{
			network->close( netattr );
			continue;
		}
		
		u.msg->size = 16;
		memcpy( u.msg->buf, u.id, u.msg->size );
		network->write( netattr, u.msg, 1 );
		break;
	}
    }
    
    memset( u.msg->buf, '\x00', u.msg->size );
    memset( v.msg->buf, '\x00', v.msg->size );
    
    /* Start ncurses */
    initscr();
    cbreak();
    noecho();

    if( showStat )
    {
    	//why is this flag needed? isnt the stat bar going to be there no matter what?
	if( COLS > 60 ) showStat = TRUE;
	else
	{
	    printf( "%s: Number of columns is too small.\n60 is the minimum.", argv[ 0 ] );
	    exit( 0 );
	}
    }

    /* Prepare Stat Window */
    s.statCount = STAT_END*2;
    s.status = (char **)calloc( s.statCount, sizeof( char ** ) );
    for( i = 0; i < s.statCount; i++ ) s.status[ i ] = calloc( WIN_STAT_W, 1 );

    snprintf( s.status[ STAT_USERS*2 ], WIN_STAT_W, "Users:" );
    snprintf( s.status[ STAT_USERS*2+1 ], WIN_STAT_W, "%s & %s", u.id, v.id );
    snprintf( s.status[ STAT_SCROLL*2 ], WIN_STAT_W, "Scroll:" );
    
    s.status = network->prep_stat( s.status, &s.statCount, WIN_STAT_W );
    network->update_stat( s.status, netattr, WIN_STAT_W );

    
    if( cipher->prep_stat != NULL )
    {
    	s.status = cipher->prep_stat( s.status, &s.statCount, WIN_STAT_W );
    	cipher->update_stat( s.status, s.statCount, ctx, WIN_STAT_W );
    }
    
    /* Create windows */
    chatWin = newwin( WIN_CHAT_H, WIN_CHAT_W, WIN_CHAT_Y, WIN_CHAT_X );
    msgWin = newwin( WIN_MSG_H, WIN_MSG_W, WIN_MSG_Y, WIN_MSG_X );
    statWin = newwin( WIN_STAT_H, WIN_STAT_W, WIN_STAT_Y, WIN_STAT_X );
    titleWin = newwin( 1, COLS, 0, 0 );
    nodelay( msgWin, TRUE );
    keypad( msgWin, TRUE );
    
    /* Add borders*/
    box( chatWin, 0, 0 );
    box( msgWin, 0, 0 );
    box( statWin, 0, 0 );

    /* Update screen */
    wrefresh( chatWin );
    wrefresh( msgWin );
    wrefresh( statWin );
    wrefresh( titleWin );
    
    printTitle( titleWin );
    updateStatus( &s, statWin );

    while( c = mvwgetch( msgWin, 1, n + 2 ) )
    {
	getyx( msgWin, s.y, s.x );
	switch( c )
	{
	    case ERR:
		if( network->read( netattr, v.msg, MSG_SIZE, 0 ) > 0 )
		{
		    if( cipher->decrypt != NULL )
		        v.msg = (struct vector *)cipher->decrypt( v.msg, ctx );

                    #ifdef DEBUG
                    	d_printf( "Main: Message recieved:", v.msg->buf, v.msg->size );
                    #endif

		    network->update_stat( s.status, netattr, s.statCount );
		    
		    if( cipher->update_stat != NULL )
		    	cipher->update_stat( s.status, s.statCount, ctx, WIN_STAT_W );
		    
		    updateBuf( &s, &v );
		    printBuf( &s, &v, chatWin );
		    updateStatus( &s, statWin );
		    //bzero( v.msg, strlen( v.msg ) );
		}
		else if( i == -1 )
		    goto end;
		break;
	    case '\n':
		if( u.msg->buf[ 0 ] == '/' )
		{
			/* Various Commands */
			if( strcmp( u.msg->buf, CMD_EXIT ) == 0 )
				goto end;
			else if( strcmp( u.msg->buf, CMD_PING ) == 0 )
			{
				//sprintf( cmd, "ping %s", ip );
				//cmd = system( cmd );
				//printMsg( cmd );
			}
			else if( strstr( u.msg->buf, CMD_ENC ) )
			{
				/* Good way of handling messages before this should be used (init probs) */
				/*
				strtok( u.msg->buf, " \t" );
				
				if( temp[ 1 ] = strtok( NULL, " \t" ) )
				{
					free( ctx );
					
					for( i = 0; ciphers[i].name != NULL; i++ )
						if( strcmp( ciphers[i].name, temp[ 1 ] ) == 0 )
						{
							cipher = &ciphers[ i ];
							break;
						}
			
					if( cipher->init != NULL )
						ctx = cipher->init( temp[ 0 ] = strtok( NULL, " \t\n" ) );
				}
				*/
			}
			else
			{
				//clrMsgBox();
				//printMsg( "" );
				//updateBuf( &s, (struct vector *){ "Problem avec le dumbass", strlen( "Problem avec le dumbass" ) } );
			}
		}
		else if( u.msg->buf[ 0 ] )
		{
                    u.msg->size = strlen( u.msg->buf );
    
		    updateBuf( &s, &u );
		    printBuf( &s, &u, chatWin );
		    
                    #ifdef DEBUG
                    	d_printf( "Main: Message to send:", u.msg->buf, u.msg->size );
                    #endif
		    
		    /* Encrypt message, if needed */
                    if( cipher->encrypt != NULL )
		    	u.msg = ( struct vector *)cipher->encrypt( u.msg, ctx );
		
		    /* Send message to peer */
		    network->write( netattr, u.msg, 0 );
		    
		    #ifdef DEBUG
                    	d_printf( "Main: Message sent:", u.msg->buf, u.msg->size );
                    #endif
		    
		    /* Update Status */
		    snprintf( s.status[ STAT_SCROLL*2 + 1 ], WIN_STAT_W, "%d%%", 100 - ( s.end - WIN_CHAT_H ) / s.free * 100 );
		    network->update_stat( s.status, netattr, s.statCount );
		    if( cipher->update_stat != NULL )
		    	cipher->update_stat( s.status, s.statCount, ctx, WIN_STAT_W );
		
		    updateStatus( &s, statWin );
		}

                //clears the msgBox after a command aswell
                clrMsgBox( msgWin );
		memset( u.msg->buf, '\x00', u.msg->size );
		n = 0;

		break;
	    case KEY_LEFT:
		if( n > WIN_MSG_X )
		    wmove( msgWin, --n, 1 );
		break;
	    case KEY_RIGHT:
		if( n < strlen( u.msg->buf ) )
		    wmove( msgWin, ++n, 1 );
		break;
	    case KEY_UP:
		if( s.end >= WIN_CHAT_H )
		{
		    s.end--;
		    printBuf( &s, &u, chatWin );
		    snprintf( s.status[ STAT_SCROLL*2+1 ], WIN_STAT_W, "%d%%", 100 - ( s.end - WIN_CHAT_H ) / s.free * 100 );
		    updateStatus( &s, statWin );
		    wrefresh( chatWin );
		}
		break;
	    case KEY_DOWN:
		if( s.end < s.free )
		{
		    s.end++;
		    printBuf( &s, &u, chatWin );
		    snprintf( s.status[ STAT_SCROLL*2+1 ], WIN_STAT_W, "%d%%", 100 - ( s.end - WIN_CHAT_H ) / s.free * 100 );
		    updateStatus( &s, statWin );
		    wrefresh( chatWin );
		}
		break;
	    case KEY_BACKSPACE:
	    case 0x7f:
		if( n > WIN_MSG_X )
		{
		    shift( u.msg->buf, n--, TRUE );
		    clrMsgBox( msgWin );
		    mvwprintw( msgWin, 1, 2, "%s", u.msg->buf );
		    updateStatus( &s, statWin );
		    wrefresh( msgWin );
		}
		break;
	    case 0x18: /* ^X */
		goto end;

	    default:
	        if( ( i = strlen( u.msg->buf ) ) < MSG_SIZE );
		{
		    if( i >= u.msg->size )
		    {
		      u.msg->buf = realloc( u.msg->buf, MSG_SIZE );
		      memset( u.msg->buf + u.msg->size, '\x00', MSG_SIZE - u.msg->size );
		      u.msg->size = MSG_SIZE;
		    }
			
		    if( n < i ) shift( u.msg->buf, n, FALSE );
		    u.msg->buf[ n++ ] = c;
		    updateStatus( &s, statWin );
		    mvwaddstr( msgWin, 1, 2, u.msg->buf );
		    wrefresh( msgWin );
	        }
	}
    }
    
    /* END */
    end:
	network->close( netattr );
	endwin();
	
	return 0;
}

int printBuf( struct session *s, struct user *u, WINDOW* chatWin )
{
    int i;
    int pos;

    wclear( chatWin );
    box( chatWin, 0, 0 );

    if( WIN_CHAT_H > s->end ) pos = 0;
    else pos = s->end - ( WIN_CHAT_H - 2 );

    for( i = 0; i < WIN_CHAT_H - 2 && s->buf[ pos + i ]; i++ )
    {
	if( s->buf[ pos + i ][ 0 ] == '%' )
	{
	    wattron( chatWin, A_BOLD );
	    mvwprintw( chatWin, i + 1, 2, "%s:", s->buf[ pos + i ] + 1 );
	    wattroff( chatWin, A_BOLD );
	}
	else
        {
	    mvwprintw( chatWin, i + 1, 2, " %s", s->buf[ pos + i ] );
        }
    }
    wrefresh( chatWin );
    return 0;
}

int updateBuf( struct session *s, struct user *u )
{
    //dude could you please replace this with a linked list structure or something better
    //method needs revising, espeacially because of potential seg fault( free > BUF_SIZE )
    s->total += u->msg->size;
    //calloc it dude?!? it nullifies the memory
    s->buf[ s->free ] = (char *)calloc( strlen( u->id ) + 1, 1 );
    //strcpy( s->buf[ s->free ], "%" );
    //better?
    *s->buf[ s->free ] = '%';
    strcat( s->buf[ s->free++ ], u->id );
    s->buf[ s->free ] = (char *)calloc( u->msg->size + 1, 1 );
    memcpy( s->buf[ s->free++ ], u->msg->buf, u->msg->size );//memcpy?
    s->end = s->free;
    return 0;
}

void printMsg( WINDOW *titleWin, char *str, int type )
{
    wmove( titleWin, 0, 2 );
    wclrtoeol( titleWin );
    attron( A_REVERSE );
    mvprintw( 0, 2, "%s", str );
    attroff( A_REVERSE );
    wrefresh( titleWin );
}

void printTitle( WINDOW *titleWin )
{
    int i;

    wattron( titleWin, A_REVERSE );

    for( i = 0; i < COLS; i++ )
    {
	waddch( titleWin, ' ' );
    }
    
    mvwaddstr( titleWin, 0, COLS / 2 - strlen( TITLE ) / 2, TITLE );
    wattroff( titleWin, A_REVERSE );
    wrefresh( titleWin );
}

void updateStatus( struct session *s, WINDOW *statWin  )
{
    int i = 0;
    int y = 1;
    
    while( i < s->statCount )
    {
	wattron( statWin, A_BOLD );
	mvwaddstr( statWin, y++, 2, s->status[ i++ ] );
	wattroff( statWin, A_BOLD );
	wmove( statWin, y, 2 );
	wclrtoeol( statWin );
	mvwaddch( statWin, y, WIN_STAT_W - 1, ACS_VLINE );
	mvwaddstr( statWin, y++, 4, s->status[ i++ ] );
    }

    wrefresh( statWin );
}

void clrMsgBox( WINDOW *msgWin )
{
    wmove( msgWin, 1, 2 );
    wclrtoeol( msgWin );
    mvwaddch( msgWin, 1, COLS - WIN_STAT_W - 2, ACS_VLINE );
}

void shift( char *str, int index, int reverse )
{
    int i;

    if( reverse )
    {
	for( i = index; i <= strlen( str ) - 1; i++ )
	{
	    str[ i - 1] = str[ i ];
	}
	str[ strlen( str ) - 1 ] = '\0';
    }
    else
    {
	for( i = strlen( str ) - 1; i >= index; i-- )
	{
	    str[ i + 1 ] = str[ i ];
	}
    }
}



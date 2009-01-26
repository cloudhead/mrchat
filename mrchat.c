/*
 ---------------------------------------------------------
 mrchat.c
 
 By: desty and kapone
 Date: April 2003
 Part of: deskap
 
 desty@7rc.org
 kapone@7rc.org
 
 ----------------------------------------------------------
 */

#include "mrchat.h"
#define OPTIONS "-c:e:hi:k:l:n:sv"
#define CYPH_AES 2

struct Cipher ciphers[] = 
{
    { "Plaintext", NULL, NULL, NULL, NULL, NULL },
    { "AES", NULL, NULL, NULL, NULL, NULL },
    { "OTP", otp_init, otp_prep_stat, otp_update_stat, otp_encrypt, otp_decrypt },
    { 0, 0, 0, 0, 0, 0 }
};

struct Network networks[] = 
{
    { "tcp", tcp_prep_stat, tcp_update_stat, tcp_connect, tcp_listen, tcp_close, tcp_write, tcp_read },
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

struct Network  *network;
struct Cipher   *cipher;

int main( int argc, char *argv[] )
{
    void *ctx = NULL;
    void *netattr = NULL;
    struct session  s;      /* Chat session */
    struct user     u;      /* Local user */
    struct user     v;      /* Peer user */
    struct Cipher   *cipher = &ciphers[ 0 ];
    struct Network  *network = &networks[ 0 ];
    
    char ans, arg;
    char *temp[ 2 ] = { NULL };
    int c, i = 1, n = 0;
    int show_stat = FALSE;
    char verbose_flag = FALSE;
    int connect_flag = FALSE;
    int option_index;
    
    WINDOW *chatwin;   /* Main window */
    WINDOW *msgwin;    /* Message box */
    WINDOW *statwin;   /* Status window */
    WINDOW *titlewin;  /* Title bar */
    
    cipher = &ciphers[ 0 ];
    network = &networks[ 0 ];
    
    /* Nullify session */
    memset( &s, 0, sizeof( s ) );
    bzero( &u, sizeof( struct user ) );
    bzero( &v, sizeof( struct user ) );
    u.msg = calloc( sizeof( struct vector ), 1 );
    u.msg->size = MSG_SIZE;
    u.msg->buf = calloc( MSG_SIZE, 1 );
    v.msg = calloc( sizeof( struct vector ), 1 );
    v.msg->size = MSG_SIZE;
    v.msg->buf = calloc( MSG_SIZE, 1 );
    
    printf( "mrchat v0.8 initialized;\n" );
    
    while( ( arg = getopt_long( argc, argv, OPTIONS, long_options, &option_index ) ) != EOF )
    {
        switch( arg )
        {
            case 'c':
                connect_flag = 1;
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
                printf( "mrchat:\n\t-c ip [port](default:1337)\n\t-e cipher to be used(default:%s)\n", ciphers[0].name );
                
                for( i = 0; ciphers[ i ].name != NULL;i++ )
                    printf( "\t\t%s\n", ciphers[ i ].name );
                printf( "\t-i nick to be used\n\t-k key to use\n\t-l [port](default:1337)\n\t-n Connection type(default:%s)\n", networks[0] );
                
                for( i = 0; networks[ i ].name != NULL; i++ )
                    printf( "\t\t%s\n", network[ i ].name );
                exit( 0 );
            case 'i': strcpy( u.id, optarg ); break;
            case 'k': temp[ 0 ] = optarg; break;
            case 'l':
                connect_flag = 0;
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
            case 's': show_stat = TRUE; break;
            case 'v': verbose_flag = TRUE; break;
            default : printf( "mrchat: invalid argument: '%c\n'", arg ); exit( 0 );
        }
    }
    
    if( cipher->init != NULL )
        ctx = (void *)cipher->init( temp[ 0 ] );
    
    /* Client */
    if( connect_flag )
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
            
            /* Read peer username */
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
    
    memset( u.msg->buf, 0, u.msg->size );
    memset( v.msg->buf, 0, v.msg->size );
    
    /* Start ncurses */
    initscr();
    cbreak();
    noecho();
    
    if( show_stat )
    {
        if( COLS > 60 ) show_stat = TRUE;
        else
        {
            printf( "%s: Number of columns is too small.\n60 is the minimum.", argv[ 0 ] );
            exit( 0 );
        }
    }
    
    /* Prepare Stat Window */
    s.stat_count = STAT_END * 2;
    s.status = (char **)calloc( s.stat_count, sizeof( char ** ) );
    for( i = 0; i < s.stat_count; i++ ) s.status[ i ] = calloc( WIN_STAT_W, 1 );
    
    snprintf( s.status[ STAT_USERS * 2 ], WIN_STAT_W, "Users:" );
    snprintf( s.status[ STAT_USERS * 2 + 1 ], WIN_STAT_W, "%s & %s", u.id, v.id );
    snprintf( s.status[ STAT_SCROLL * 2 ], WIN_STAT_W, "Scroll:" );
    
    s.status = network->prep_stat( s.status, &s.stat_count, WIN_STAT_W );
    network->update_stat( s.status, netattr, WIN_STAT_W );
    
    if( cipher->prep_stat != NULL )
    {
        s.status = cipher->prep_stat( s.status, &s.stat_count, WIN_STAT_W );
        cipher->update_stat( s.status, s.stat_count, ctx, WIN_STAT_W );
    }
    
    /* Create windows */
    chatwin = newwin( WIN_CHAT_H, WIN_CHAT_W, WIN_CHAT_Y, WIN_CHAT_X );
    msgwin = newwin( WIN_MSG_H, WIN_MSG_W, WIN_MSG_Y, WIN_MSG_X );
    statwin = newwin( WIN_STAT_H, WIN_STAT_W, WIN_STAT_Y, WIN_STAT_X );
    titlewin = newwin( 1, COLS, 0, 0 );
    nodelay( msgwin, TRUE );
    keypad( msgwin, TRUE );
    
    /* Add borders*/
    box( chatwin, 0, 0 );
    box( msgwin, 0, 0 );
    box( statwin, 0, 0 );
    
    /* Update screen */
    wrefresh( chatwin );
    wrefresh( msgwin );
    wrefresh( statwin );
    wrefresh( titlewin );
    
    print_title( titlewin );
    update_status( &s, statwin );
    
    while( c = mvwgetch( msgwin, 1, n + 2 ) )
    {
        getyx( msgwin, s.y, s.x );
        switch( c )
        {
            case ERR:
                if( network->read( netattr, v.msg, MSG_SIZE, 0 ) > 0 )
                {
                    if( cipher->decrypt != NULL )
                        v.msg = (struct vector *)cipher->decrypt( v.msg, ctx );
                    
                    network->update_stat( s.status, netattr, s.stat_count );
                    
                    if( cipher->update_stat != NULL )
                        cipher->update_stat( s.status, s.stat_count, ctx, WIN_STAT_W );
                    
                    update_buf( &s, &v );
                    print_buf( &s, &v, chatwin );
                    update_status( &s, statwin );
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
                        
                    }
                    else if( strstr( u.msg->buf, CMD_ENC ) )
                    {
                        
                    }
                    else
                    {
                    }
                }
                else if( u.msg->buf[ 0 ] )
                {
                    u.msg->size = strlen( u.msg->buf );
                    update_buf( &s, &u );
                    print_buf( &s, &u, chatwin );
                    
                    /* Encrypt message, if needed */
                    if( cipher->encrypt != NULL )
                        u.msg = ( struct vector *)cipher->encrypt( u.msg, ctx );
                    
                    /* Send message to peer */
                    network->write( netattr, u.msg, 0 );
                    
                    /* Update Status */
                    snprintf( s.status[ STAT_SCROLL * 2 + 1 ], WIN_STAT_W, "%d%%", 100 - ( s.end - WIN_CHAT_H ) / s.free * 100 );
                    network->update_stat( s.status, netattr, s.stat_count );
                    
                    if( cipher->update_stat != NULL )
                        cipher->update_stat( s.status, s.stat_count, ctx, WIN_STAT_W );
                    
                    update_status( &s, statwin );
                }
                clr_msgbox( msgwin );
                memset( u.msg->buf, 0, u.msg->size );
                n = 0;
                break;
            case KEY_LEFT:
                if( n > WIN_MSG_X )
                    wmove( msgwin, --n, 1 );
                break;
            case KEY_RIGHT:
                if( n < strlen( u.msg->buf ) )
                    wmove( msgwin, ++n, 1 );
                break;
            case KEY_UP:
                if( s.end >= WIN_CHAT_H )
                {
                    s.end--;
                    print_buf( &s, &u, chatwin );
                    snprintf( s.status[ STAT_SCROLL * 2 + 1 ], WIN_STAT_W, "%d%%", 100 - ( s.end - WIN_CHAT_H ) / s.free * 100 );
                    update_status( &s, statwin );
                    wrefresh( chatwin );
                }
                break;
            case KEY_DOWN:
                if( s.end < s.free )
                {
                    s.end++;
                    print_buf( &s, &u, chatwin );
                    snprintf( s.status[ STAT_SCROLL * 2 + 1 ], WIN_STAT_W, "%d%%", 100 - ( s.end - WIN_CHAT_H ) / s.free * 100 );
                    update_status( &s, statwin );
                    wrefresh( chatwin );
                }
                break;
            case KEY_BACKSPACE:
            case 0x7f:
                if( n > WIN_MSG_X )
                {
                    shift( u.msg->buf, n--, TRUE );
                    clr_msgbox( msgwin );
                    mvwprintw( msgwin, 1, 2, "%s", u.msg->buf );
                    update_status( &s, statwin );
                    wrefresh( msgwin );
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
                    memset( u.msg->buf + u.msg->size, 0, MSG_SIZE - u.msg->size );
                    u.msg->size = MSG_SIZE;
                }
                
                if( n < i ) shift( u.msg->buf, n, FALSE );
                u.msg->buf[ n++ ] = c;
                update_status( &s, statwin );
                mvwaddstr( msgwin, 1, 2, u.msg->buf );
                wrefresh( msgwin );
            }
        }
    }
    
    /* END */
end:
    network->close( netattr );
    endwin();
    
    return 0;
}

int print_buf( struct session *s, struct user *u, WINDOW* chatwin )
{
    int i;
    int pos;
    
    wclear( chatwin );
    box( chatwin, 0, 0 );
    
    if( WIN_CHAT_H > s->end ) pos = 0;
    else pos = s->end - ( WIN_CHAT_H - 2 );
    
    for( i = 0; i < WIN_CHAT_H - 2 && s->buf[ pos + i ]; i++ )
    {
        if( s->buf[ pos + i ][ 0 ] == '%' )
        {
            wattron( chatwin, A_BOLD );
            mvwprintw( chatwin, i + 1, 2, "%s:", s->buf[ pos + i ] + 1 );
            wattroff( chatwin, A_BOLD );
        }
        else
        {
            mvwprintw( chatwin, i + 1, 2, " %s", s->buf[ pos + i ] );
        }
    }
    wrefresh( chatwin );
    return 0;
}

int update_buf( struct session *s, struct user *u )
{
    /*
        TODO: Convert to linked-list
    */
    s->total += u->msg->size;
    s->buf[ s->free ] = calloc( strlen( u->id ) + 1, 1 );
    *s->buf[ s->free ] = '%';
    strcat( s->buf[ s->free++ ], u->id );
    s->buf[ s->free ] = calloc( u->msg->size + 1, 1 );
    memcpy( s->buf[ s->free++ ], u->msg->buf, u->msg->size );//memcpy?
    s->end = s->free;
    
    return 0;
}

void print_msg( WINDOW *titlewin, char *str, int type )
{
    wmove( titlewin, 0, 2 );
    wclrtoeol( titlewin );
    attron( A_REVERSE );
    mvprintw( 0, 2, "%s", str );
    attroff( A_REVERSE );
    wrefresh( titlewin );
}

void print_title( WINDOW *titlewin )
{
    int i;
    
    wattron( titlewin, A_REVERSE );
    
    for( i = 0; i < COLS; i++ )
    {
        waddch( titlewin, ' ' );
    }
    
    mvwaddstr( titlewin, 0, COLS / 2 - strlen( TITLE ) / 2, TITLE );
    wattroff( titlewin, A_REVERSE );
    wrefresh( titlewin );
}

void update_status( struct session *s, WINDOW *statwin  )
{
    int i = 0;
    int y = 1;
    
    while( i < s->stat_count )
    {
        wattron( statwin, A_BOLD );
        mvwaddstr( statwin, y++, 2, s->status[ i++ ] );
        wattroff( statwin, A_BOLD );
        wmove( statwin, y, 2 );
        wclrtoeol( statwin );
        mvwaddch( statwin, y, WIN_STAT_W - 1, ACS_VLINE );
        mvwaddstr( statwin, y++, 4, s->status[ i++ ] );
    }
    
    wrefresh( statwin );
}

void clr_msgbox( WINDOW *msgwin )
{
    wmove( msgwin, 1, 2 );
    wclrtoeol( msgwin );
    mvwaddch( msgwin, 1, COLS - WIN_STAT_W - 2, ACS_VLINE );
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



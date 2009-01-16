/***************************************************************************
 *   Copyright (C) 2004 by Desty and Kapone                                *
 *   root@kapone                                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "tcp.h"

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

void print_flags( int s )
{
	long temp = fcntl( s, F_GETFL );
	
	printf( "0x%x\tO_NONBLOCK:%x\tO_ASYNC:%x\n", temp, temp & O_NONBLOCK, temp & O_ASYNC );
}

char** tcp_prep_stat( char **stat, int *statCount, int size )
{
	int i = *statCount;
	
	*statCount += TCP_NORMAL_END * 2;
	if( ( stat = (char**)realloc( stat, (*statCount)*sizeof(char**) ) ) == NULL )
		exit( -1 );
	
	stat[ (STAT_END+TCP_NORMAL_PEER) * 2 ] = "Peer IP:";
	stat[ (STAT_END+TCP_NORMAL_PORT) * 2 ] = "Port:";

	for(; i < *statCount; i += 2 )
		stat[ i + 1 ] = calloc( size, 1 );

	return stat;
}

int tcp_close( struct tcp_attr *attr )
{
	close( attr->sock );
	free( attr );
	
	return 1;
}

struct tcp_attr *tcp_connect( char *sAttr )
{
	struct tcp_attr *attr = calloc( 1, sizeof( struct tcp_attr ) );
	char* ip_address = calloc( 16, 1 );
	char *temp_, *temp = calloc( 22, 1 );
	unsigned short port = PORT;

	printf( "tcp_connect:\n" );

	temp_ = temp;
	
	if( sAttr == NULL )
	{
		printf( "Enter IP [PORT]:" );
		scanf( "%21s", temp );
		
		ip_address = strtok( temp, " \t" );
		if( ( temp = strtok( temp, " \t" ) ) != NULL )
			port = atoi( temp );
		else
			printf( "Using PORT:%i\n", port );
	}
	else
		sscanf( sAttr, "%15s %i", ip_address, &port );
        
	free(temp_);
        
	attr->local.sin_addr.s_addr = INADDR_ANY;
	attr->local.sin_family = attr->peer.sin_family = AF_INET;
	attr->local.sin_port = attr->peer.sin_port = htons( port );
	
	if( attr->local.sin_addr.s_addr == INADDR_NONE )
	{
		perror( "\tNo local IP allocated\n" );
		return NULL;
	}
	else
		printf( "\tLocal %s as local address\n", inet_ntoa( attr->local.sin_addr ) );
	
	/* Create Socket */
	if( ( attr->sock = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	{
		perror( "\tSocket() error\n" );
		return NULL;
	}
	printf( "\tSock created\n" );

	if( INADDR_NONE == ( attr->peer.sin_addr.s_addr = inet_addr( sAttr ) ) )
	{
		printf( "\t!!%s not a valid address!!\n", sAttr );
		free( attr );
		exit( -1 );
	}

	/* Connect to peer */
	printf( "\tConnecting to %s:%i\n", sAttr, port );
	if( connect( attr->sock, ( struct sockaddr* )&attr->peer, sizeof( attr->peer ) ) )
	{
	    perror( "\t!!connect() error!!\n" );
	    return NULL;
	}

	printf( "\tconnected to %s\n", inet_ntoa( attr->peer.sin_addr ) );

	print_flags( attr->sock );
	return attr;
}

struct tcp_attr *tcp_listen( char *sAttr )
{
	struct tcp_attr *attr = calloc( 1, sizeof( struct tcp_attr ) );
	int i = 0;
	int rc = 0;
	unsigned short port = PORT;

	printf( "tcp_listen:\n" );

	if( sAttr != NULL )
		sscanf( sAttr, "%i", &port );
	else
		printf( "Using PORT:%i\n", port );

	attr->local.sin_addr.s_addr = INADDR_ANY;
	attr->local.sin_family = attr->peer.sin_family = AF_INET;
	attr->local.sin_port = attr->peer.sin_port = htons( port );
	
	if( attr->local.sin_addr.s_addr == INADDR_NONE )
	{
		perror( "\tNo local IP allocated\n" );
		return NULL;
	}
	else
		printf( "\tListening on %s:%i\n", attr->local.sin_addr.s_addr, port );
	
	/* Create Socket */
	if( ( attr->sock = socket( AF_INET, SOCK_STREAM, 0 ) ) < 0 )
	{
		perror( "\tsocket() error\n" );
		exit( -1 );
	}
	printf( "\tSock created\n" );

	if( bind( attr->sock, (struct sockaddr*)&attr->peer, sizeof( attr->peer ) ) )
	{
		perror( "\tBind error\n" );
		goto exit;
	}

	if( listen( attr->sock, 1 ) )
	{
		perror( "\tlisten error\n" );
		goto exit;
	}

	printf( "\tListening...\n" );
	i = sizeof( attr->peer );
	if( ( rc = accept( attr->sock, ( struct sockaddr* )&attr->peer, &i ) ) < 0 )
	{
		perror( "\tAccept error\n" );
		goto exit;
	}
	
	close( attr->sock );
	
	/* Set to non-blocking( Linux ) */
	print_flags( rc );
	

	attr->sock = rc;
	printf( "\tConnection from %s\n", inet_ntoa( attr->peer.sin_addr ) );

	return attr;
	
exit:
	close( attr->sock );
	free( attr );
	return NULL;
}

int tcp_read( struct tcp_attr *attr, struct vector *input, unsigned int maxsize, char opts )
{
    unsigned int i = 0;
    int r, check;
    struct timeval tv;

    fd_set sock;
    FD_SET( attr->sock, &sock );
    memset( &tv, '\x00', sizeof( tv ) );

    if( input->size < maxsize )
    	input->buf = realloc( input->buf, maxsize );

    if( !opts )
    {
		for( i = 0; ( check = select( attr->sock + 1, &sock, NULL, NULL, &tv ) ) > 0
			&& ( r = read( attr->sock, &input->buf[ i ], maxsize - i ) ) > 0; i += r );
		
		if( check == -1 )
		{
			switch( errno )
			{
				case EBADF:
					printf( "Invalid File Descriptor" );
					break;
				case EINTR:
					printf( "A signal was caught" );
					break;
				case ENOMEM:
					printf( "Unabe to allocate fuckign memory" );
					break;
				default:
					break;
			}
		}
		if( r == 0 ) return -1;
    }
    else
    {
    	for( i = 0; i < maxsize 
		&& ( r = read( attr->sock, &input->buf[ i ], maxsize - i ) ) > 0; i += r );
		if( r == 0 ) return -1;
    }

    if( i > 0 )
    {
    	input->size = i;
    	input->buf = realloc( input->buf, i );
    }

    return i;
}

int tcp_write( struct tcp_attr *s, struct vector *input, char opts )
{
   return write( s->sock, input->buf, input->size );
}

int tcp_update_stat( char** stat, void *pattr, int size )
{
	struct tcp_attr *attr = pattr;
	
	/* Client IP */
	strncpy( stat[ (STAT_END + TCP_NORMAL_PEER)*2 + 1 ], inet_ntoa( attr->peer.sin_addr ), size );

	/* Port */
	snprintf( stat[ (STAT_END + TCP_NORMAL_PORT)*2 + 1 ], size, "%i", ntohs( attr->local.sin_port ) );

	return 0;
}

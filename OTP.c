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
#include "common.h"
#include "OTP.h"

char** OTP_prep_stat( char **stat, int *statCount, int size )
{
	*statCount += OTP_END*2;
	
	if( ( stat = (char **)realloc( stat, (*statCount)*sizeof( char** ) ) ) == NULL )
		exit( -1 );
	
	
	stat[ *statCount - ( OTP_END - OTP_FILE )*2 ] = "FILE:";
	stat[ *statCount - ( OTP_END - OTP_FILE )*2 + 1 ] = calloc( size, 1 );
	stat[ *statCount - ( OTP_END - OTP_POS )*2 ] = "Pos:";
	stat[ *statCount - ( OTP_END - OTP_POS )*2 + 1] = calloc( size, 1 );
	stat[ *statCount - ( OTP_END - OTP_SIZE )*2 ] = "Size:";
	stat[ *statCount - ( OTP_END - OTP_SIZE )*2 + 1] = calloc( size, 1 );
	
	return stat;
}


struct OTP_key *OTP_init( char* keyFile )
{
    FILE *fpKey;
    struct OTP_key *key = (struct OTP_key *)calloc( 1, sizeof( struct OTP_key ) );

    printf( "::One Time Pad::\n" );
    
    key->file = keyFile;
    //key->file = calloc( strlen( keyFile ) + 1, 1 );
    //memcpy( key->file, keyFile, strlen( keyFile ) );
    key->pos = 0;
    
    /* Check for Key */
    if( key->file == NULL || ( fpKey = fopen( key->file, "r" ) ) == NULL )
    {
    	for(;;)
	{
    		printf( "Enter key file:" );
		scanf( "%s", &key->file );
		if( ( fpKey = fopen( key->file, "r" ) ) != NULL )
			break;
		printf( "No such file:%s\n", key->file );
		free( keyFile );
	}
    }
    else
    	printf( "\tUsing key file: %s\n", key->file );

    key->size = getSize( fpKey );
    printf( "\tKey Size:%i\n", key->size );
    
    #ifdef DEBUG
    	f_printf( "OTP Initialization" );
    	i_printf( "\tKey Size:", key->size );
    #endif

    /* Shutdown */
    fclose( fpKey );
    
    return key;
}

char getChar( FILE* fpKey, long long int pos )
{
	fseek( fpKey, pos, SEEK_SET );
	
	return getc( fpKey );
}

struct vector *OTP_encrypt( struct vector *input, struct OTP_key *ctx )
{
        int i;
	FILE *fpKey;

	fpKey = fopen( ctx->file, "r" );
	
	#ifdef DEBUG
		d_printf( "OTP::OTP_encrypt", input->buf, input->size );
	#endif
	
	for( i = 0; i < input->size; i++ )
	{
		input->buf[ i ] = ( input->buf[ i ] + getChar( fpKey, ctx->pos++ ) ) % 256;
	}

	fclose( fpKey );
	
        return input;
}

struct vector *OTP_decrypt( struct vector *input, struct OTP_key *ctx )
{
        int i;
	FILE *fpKey = fopen( ctx->file, "r" );
	
	for( i = 0; i < input->size; i++ )
	{
		input->buf[ i ] = ( input->buf[ i ] - getChar( fpKey, ctx->pos++ ) ) % 256;
	}
	
	if( ctx->pos == ctx->size ) exit( -1 );

	fclose( fpKey );
	
        return input;
}

int OTP_update_stat( char** stat, int statCount, void *pattr, int size )
{
	struct OTP_key *ctx = (struct OTP_key *)pattr;

	snprintf( stat[ statCount - (OTP_END - OTP_FILE)*2 + 1 ], size, "%s", ctx->file );
	snprintf( stat[ statCount - (OTP_END - OTP_POS)*2 + 1 ], size, "%i", ctx->pos );
	snprintf( stat[ statCount - (OTP_END - OTP_SIZE)*2 + 1 ], size, "%i", ctx->size );
	
	return 0; 
}
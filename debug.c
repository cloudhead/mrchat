/***************************************************************************
 *   Copyright (C) 2004 by Desty & Kapone                                  *
 *   root@kapone                                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "common.h"

#ifdef DEBUG

void output_ascii( FILE* file, unsigned char *buf, int size )
{
	int i;

	for( i = 0; i < size; i++ )
	{
        	if( !(i%10) ){ putc( '\n', file ); putc( '\t', file ); }
		fprintf( file, "|%c|", (unsigned char)buf[i] );
	}

        putc( '\n', file );
}

void output_hex( FILE* file, unsigned char *buf, int size )
{
	int i;

	for( i = 0; i < size; i++ )
	{
        	if( !(i%10) ){ putc( '\n', file ); putc( '\t', file ); }
		fprintf( file, "|%2x|", (unsigned char)buf[i] );
	}

        putc( '\n', file );
}

void d_printf( char* fstring, char* data, int size )
{
	FILE* file = fopen( logFile, "a" );

	fprintf( file, "%s\n\tBytes:%i\n", fstring, size );

        output_ascii( file, data, size );

        output_hex( file, data, size );

        fclose( file );
}

void i_printf( char* fstring, unsigned long long int i )
{
	FILE* file = fopen( logFile, "a" );

        fprintf( file, "%s %i\n", fstring, i );

        fclose( file );
}

f_printf( char* fstring )
{
	FILE* file = fopen( logFile, "a" );

        fprintf( file, "%s\n", fstring );

        fclose( file );
}

#endif

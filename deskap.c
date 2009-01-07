/*
---------------------------------------------------------
Project: deskap 1.0
Filename: deskap.c
By: desty and kapone
Date: April 2003
Part of: mrchat
License: GNU
---------------------------------------------------------
*/

#include "deskap.h"

int fake_log( int input )
{
	if( input == 0 )
        {
        	printf( "fake_log: input 0\n" );
                exit( -1 );
        }

	unsigned int i, temp = 1 << ( sizeof( unsigned int ) * 8 - 1 );

        for( i = 0; !( ( temp >> i ) & input ); i++ );

        return sizeof( unsigned int )*8 - i - 1;
}

char** deskap_prep_stat( char **stat, int *statCount, int size )
{
	*statCount += DESKAP_END*2;
	
	if( ( stat = (char **)realloc( stat, (*statCount)*sizeof( char** ) ) ) == NULL )
		exit( -1 );
	
	stat[ *statCount - ( DESKAP_END - DESKAP_FILE )*2 ] = "FILE:";
	stat[ *statCount - ( DESKAP_END - DESKAP_FILE )*2 + 1 ] = calloc( size, 1 );
	stat[ *statCount - ( DESKAP_END - DESKAP_CHARS_SENT )*2 ] = "Chars Sent:";
	stat[ *statCount - ( DESKAP_END - DESKAP_CHARS_SENT )*2 + 1] = calloc( size, 1 );
	stat[ *statCount - ( DESKAP_END - DESKAP_SIZE )*2 ] = "Size:";
	stat[ *statCount - ( DESKAP_END - DESKAP_SIZE )*2 + 1] = calloc( size, 1 );
	
	return stat;
}

struct deskap_key *deskap_init( char* keyFile )
{
    static struct deskap_key ctx;
    FILE *fpKey;

    printf( "::DESKAP::\n" );
    
    ctx.file = keyFile;
    
    /* Check for Key */
    if( ctx.file == NULL || ( fpKey = fopen( ctx.file, "r" ) ) == NULL )
    {
    	for(;;)
	{
    		printf( "Enter key file:" );
		scanf( "%as", &ctx.file );
		if( ( fpKey = fopen( ctx.file, "r" ) ) != NULL )
			break;
		printf( "No such file:%s\n", ctx.file );
		free( keyFile );
	}
    }
    else
    	printf( "\tUsing key file: %s\n", ctx.file );


    ctx.fileLength = getSize( fpKey );
    
    /* Allocate space for key */
    ctx.key = ( unsigned char *)malloc( ctx.fileLength );
    printf( "\tKey Size:%i\n", ctx.fileLength );
    //if( verboseFlag ) printf("%s: Loading key: %s\n", argv[ 0 ], keyFile );

    //Maximum lenght in bits
    ctx.bLength = (char)fake_log( ctx.fileLength ) + 1;
    printf( "\tBit Length:%i\n", ctx.bLength );

    #ifdef DEBUG
    	f_printf( "Deskap Initialization" );
    	i_printf( "\tKey Size:", ctx.fileLength );
        i_printf( "\tBit Length:", (int)ctx.bLength );
    #endif

    /* Load key */
    printf( "\tKey loading!! This may take a couple of min!!!\n" );
    loadKey( fpKey, &ctx );
    printf( "\tKey Loaded\n" );

    /* Shutdown */
    fclose( fpKey );

    return &ctx;
}


struct vector *deskap_encrypt( struct vector *input, struct deskap_key *ctx )
{
        unsigned long int x; /* Main variable */
        int bPlace = 0;	//the overallposition in bits, in the buffer
        int blPlace;	//keeps track of how far we are in the integer we are concatenating on

	struct vector *output = (struct vector *)calloc( 1, sizeof( struct vector ) );
        output->buf = (unsigned char *)calloc( ( input->size * ctx->bLength )/8 + 1, 1 );

        for( output->size = 0; output->size < input->size; output->size++ )
        {
                /* Main Algorithm */
		if( /* output->buf[ output->size ] < ASCII && */ ctx->charCount[ input->buf[ output->size ] ] )
			x = ctx->charPos[ ( int )input->buf[  output->size ] ][ ( random() % ctx->charCount[ ( unsigned long int )input->buf[ output->size ] ] ) ];
		else
			x = random() % ctx->fileLength;

                blPlace = 0;

                //compression routine
                //encass first part of byte is already used
                if( bPlace % 8 )
                {
                        output->buf[ bPlace/8 ] |= ( x >> ( bPlace % 8 - 8 + ctx->bLength ) );
			//bPlace = (bPlace/8 + 1)*8
                	blPlace = 8 - bPlace % 8;
                }

                //no part of byte is used
                while( (ctx->bLength - blPlace) >= 8 )
                {
                	blPlace += 8;
                	output->buf[ (bPlace + blPlace)/8 - 1 ] =  x >> ( ctx->bLength - blPlace );
                }

                //final part of int added on
                if( blPlace != ctx->bLength )
                	output->buf[ (bPlace + blPlace)/8 ] = x << 8 - ( ctx->bLength + bPlace ) % 8;

                bPlace += ctx->bLength;
        }

	output->size = bPlace/8 + ( bPlace%8 > 0 );

        free( input->buf );
	free( input );

        return output;
}


struct vector *deskap_decrypt( struct vector *input, struct deskap_key *ctx )
{
        char c;	//a variable for holding, temperairily, a specific character
        unsigned long int x; /* Main variable */
        int bPlace = 0;	//the overallposition in bits, in the buffer
        int blPlace;	//keeps track of how far we are in the integer we are concatenating on

	struct vector *output = (struct vector *)calloc( 1, sizeof( struct vector ) );
        output->buf = (unsigned char *)calloc( ( input->size * 8 )/ctx->bLength + 1, 1 );

        for( output->size = 0; bPlace/8 + ( bPlace%8 > 0 ) < input->size; output->size++ )
        {
                /* Main Algorithm */
                x = 0;
                blPlace = 0;

                //decompression routine
                if( bPlace % 8 )
                {
                        blPlace += 8 - bPlace % 8;
                	x = (unsigned int)( (unsigned char)input->buf[ bPlace / 8 ] & ( 0xff >> bPlace % 8 ) ) << ( ctx->bLength - blPlace );
                }

                while( (ctx->bLength - blPlace) >= 8 )
                {
                	blPlace += 8;
                	x |= (unsigned int)(unsigned char)input->buf[ ( bPlace + blPlace )/8 - 1 ] << ( ctx->bLength - blPlace ); //- 8;  jus so you understand why
			#ifdef DEBUG
                        	i_printf( "DESKAP::deskap_decrypt:mid input->buf:", (unsigned int)(unsigned char)input->buf[ ( bPlace + blPlace )/8 - 1 ] );
			#endif
                        //blPlace += 8;
                }

                if( blPlace != ctx->bLength )
                	x |= (unsigned char)input->buf[ ( bPlace + blPlace )/8 ] >> ( 8 - ( bPlace + ctx->bLength ) % 8 );

                bPlace += ctx->bLength;

		#ifdef DEBUG
                	i_printf( "DESKAP::deskap_decrypt:x:", x );
                	d_printf( "DESKAP::deskap_decrypt:", input->buf, input->size );
		#endif
		
		if( x < ctx->fileLength )
                	output->buf[ output->size ] = ctx->key[ x ];
		else
			output->buf[ output->size ] = '_';
        }

        free( input->buf );
        free( input );

        return output;
}


unsigned long int getSize( FILE* fp )
{
    unsigned long int size;
    fseek( fp, 0, SEEK_END );
    size = ftell( fp ) - 1;
    rewind( fp );
    return size;
}

int loadKey( FILE *fp, struct deskap_key *ctx )
{
    unsigned char c;
    unsigned long int i = 0;
    int n;

    printf( "\tLoading key into array...\n" );
    /* Load the key into array */
    for( i = 0; i < ctx->fileLength; i++ )
    {
    	c = getc( fp );
	ctx->key[ i ] = c;
	ctx->charCount[ c ]++;
    }

    printf( "\tAllocating memory...\n"  );
    /* Allocate space for charPos */
    for( i = 0; i < ASCII; i++ )
    {
	ctx->charPos[ i ] = (int *)calloc( ctx->charCount[ i ], sizeof( ctx->charCount[ 0 ] ) );
    }

    printf( "\tSorting positions...\n" );
    memset( ctx->charCount, '\x00', ASCII * sizeof( ctx->charCount[ 0 ] ) );
    /* Get positions of all characters in key */
    for( i = 0; i < ctx->fileLength; i++ )
    {

	/*n = 0;
	while( n < ctx->charCount[ ( int )ctx->key[ i ] ] )
	{
	    if( !ctx->charPos[ ( int )ctx->key[ i ] ][ n ] )
	    {
		ctx->charPos[ ( int )ctx->key[ i ] ][ n ] = i;
		break;
	    }
	    else n++;
	}*/
        //if( !(i%1000) ) printf( "%i\n", i );
	ctx->charPos[ ctx->key[ i ] ][ ctx->charCount[ ctx->key[ i ] ]++ ] = i;
    }
    
    putchar( '\n' );
    return 1;
}

int deskap_update_stat( char** stat, int statCount, void *pattr, int size )
{
	struct deskap_key *ctx = (struct deskap_key *)pattr;

	snprintf( stat[ statCount - (DESKAP_END - DESKAP_FILE)*2 + 1 ], size, "%s", ctx->file );
	snprintf( stat[ statCount - (DESKAP_END - DESKAP_CHARS_SENT)*2 + 1 ], size, "N/A" );
	snprintf( stat[ statCount - (DESKAP_END - DESKAP_SIZE)*2 + 1 ], size, "%i", ctx->fileLength );
	
	return 0; 
}

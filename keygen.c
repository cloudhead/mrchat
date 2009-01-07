
#include "keygen.h"

char randChar( int[ 26 ], int );
void displayHelp( void );

int main( int argc, char *argv[] )
{
    int compressFlag = FALSE;
    int useFreq = FALSE;
    int overwriteFlag = FALSE;
    int verboseFlag = FALSE;
    
    unsigned int i, d, size = 0, j = 0;
    int freq[ 26 ];
    
    char *freqFile = NULL;
    char *outFile = NULL;
    char *outFileGz = malloc( 128 );
    char *cmd = malloc( 128 );

    FILE *fpOut, *fpFreq;

    setbuf( stdout, 0 );
    
    if( argc < 3 )
    {
        printf("dkgen: Not enough arguments\n");
        exit( 0 );
    }

    for( i = 1; i < argc; i++ )
    {
        if( argv[ i ][ 0 ] == '-' )
        {
            for( j = 1; j < strlen( argv[ i ] ); j++ )
            {
                switch( argv[ i ][ j ] )
                {
                    case 'h': displayHelp(); exit( 0 );
                    case 'V': printf( VERSION ); exit( 0 );
                    case 'v': verboseFlag = TRUE; break;
                    case 'c': compressFlag = TRUE; break;
                    case 'w': overwriteFlag = TRUE; break;
                    case 's': size = atoi( argv[ i + 1 ] ) * 1024; break;
                    case 'l': size = 1440; break;
                    case 'f': freqFile = argv[ i + 1 ]; break;
                    case 'o': outFile = argv[ i + 1 ];
                    default:
                        printf("deskap: Unrecognized argument: '%c'\n", argv[ i ][ j ] );
                        printf( HELP_MSG );
                        exit( 0 );
                }
            }
        }
    }

    if( !size )
    {
        printf( "dkgen: Warning: Size not specified, using default(%d)\n", DEF_SIZE );
        size = DEF_SIZE;
    }
    else if( verboseFlag ) printf( "dkgen: Size is: %u\n", size );
        
    if( compressFlag )
    {
        size = floor( size / 65 * 100 );
    }
    
    /* Overwrite file if it already exists */
    if( ( fpOut = fopen( outFile, "r" ) ) != NULL )
    {
        if( overwriteFlag ) remove( outFile );
        else
        {
            printf("dkgen: %s: File already exists.\nOverwrite?(y/n)", outFile );
            if( getchar() == 'y' )
            {
                remove( outFile );
                getchar();
            }
            else exit( 0 );
        }
        fclose( fpOut );
    }

    /* Create output file */
    if( ( fpOut = fopen( outFile, "a" ) ) == NULL )
    {
        printf( "dkgen: %s: Cannot create file\n", outFile );
        exit( 0 );
    }

    /* Load frequency file */
    if( freqFile )
    {
        i = 0;
        useFreq = TRUE;
        if( ( fpFreq = fopen( freqFile, "r" ) ) == NULL )
        {
            printf( "dkgen: %s: No such file\n", freqFile );
            exit( 0 );
        }
        while( fscanf( fpFreq, "%d", &d ) != EOF )
        {
            freq[ i ] = d;
            i++;
        }
        if( verboseFlag ) printf("dkgen: Frequency file loaded: %s\n", freqFile );
    }
    
    if( verboseFlag )
    {
        printf("dkgen: Output file: %s\n", outFile );
        printf("dkgen: Generating key");
    }

    /* Write key file */
    for( i = 0; i < size; i++ )
    {
        fprintf( fpOut, "%c", randChar( freq, useFreq ) );
        if( verboseFlag && ( j * 100 ) / size == 10 )
        {
            printf(".");
            j = 0;
        }
        j++;
    }
    
    if( verboseFlag ) printf(" done!\n");
        
    if( compressFlag )
    {
        /* outFileGz = outFile + ".gz" */
        strcpy( outFileGz, outFile );
        if( !strstr( outFileGz, ".gz" ) )
            strcat( outFileGz, ".gz" );

        /* Overwrite file if it exists */
        if( fopen( outFileGz, "r" ) != NULL )
        {
            if( overwriteFlag ) remove( outFileGz );
            else
            {
                printf("dkgen: %s: File already exists.\nOverwrite?(y/n)", outFileGz );
                if( getchar() == 'y' ) remove( outFileGz );
                else exit( 0 );
            }
        }
        if( verboseFlag ) printf("dkgen: Compressing: %s... ", outFile );

        /* Compress file */
        strcat( cmd, "gzip " );
        strcat( cmd, outFile );
        system( cmd );

        if( verboseFlag ) printf("done!\ndkgen: File name: %s\n", outFileGz );
    }
    

    /* Shutdown */
    free( cmd );
    free( outFileGz );
    fclose( fpOut );

    if( freqFile ) fclose( fpFreq );
    if( compressFlag ) remove( outFile );
    
    return 0;
}

char randChar( int freq[ 26 ], int useFreq )
{
    char c;
    int r, i;
    const char symbol[] = {'0','1','2','3','4','5','6','7','8','9','(',')','[',
        ']','{','}','<','>','.',',',':',';','!','?','-','_','$','#','%','+','=',
        '*','/','\\','|','`','"','\'','^','@','~','A','B','C','D','E','F','G','H',
'I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z'};
    
    if( useFreq )
    {
        r = random() % 101;

        if( r < 75 )
        {
            r = random() % 101;
            int old = 0;
            for( i = 0; i < 26; i++ )
            {
                if( old <= r && r <= ( old + freq[ i ] ) )
                {
                    c = ( char )i + 97;
                    break;
                }
                old += freq[ i ];
            }
        }
        else if( r < 95 )
        {
            c = ' ';
        }
        else
        {
            r = random() % strlen( symbol );
            c = symbol[ r ];
        }
    }
    else
    {
        r = random() % 95;
        c = ( char )( r + 32 );
    }

    return c;
}

void displayHelp( void )
{
    printf("Usage: dkgen [-Vchvw] [-q file] -s size -o file");
}

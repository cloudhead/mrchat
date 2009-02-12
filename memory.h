#ifdef MEMORY_H
#define MEMORY_H

#define MALLOC( A, B, C )	if( !( A = calloc( B, C ) ) ) exit( EXIT_FAILURE )
#define CALLOC( A, B, C )	if( !( A = calloc( B, C ) ) ) exit( EXIT_FAILURE )

#endif


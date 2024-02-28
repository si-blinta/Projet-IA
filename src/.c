#include "ia/name.h"


Name* NAME_create()
{
    // Allocation de la struture de donnees
    Name* name= (Name*)malloc( sizeof( Name ) );
    memset( name, 0, sizeof( Name ) );

	// Initialisation...
	// TODO

    return( name );
}


void NAME_destroy( Name* name )
{
    // Si valide
    if( name != NULL )
    {
        // Liberation memoire
        free( name );
    }
}

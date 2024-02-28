#include "ia/network.h"

// System
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


Network* NETWORK_create( const Config* cfg )
{
    // Allocation de la struture de donnees
    Network* network= (Network*)malloc( sizeof( Network ) );
    memset( network, 0, sizeof( Network ) );

	// Creation couche d'entree (pas de couche precedente)
    network->input = LAYER_create( network, cfg->inputSize, NULL );

    // Couche precedente de la couche en cours de creation (pour interconnexion). On initialise avec la
    // couche d'entree, puis la derniere couche creee devient la precedente de la prochaine...
    Layer* previous = network->input;

    // Creation des couches internes
    network->nbInternals = cfg->nbLayers;
    network->internals = (Layer**)malloc( network->nbInternals * sizeof( Layer* ) );
    for( uint16_t i = 0; i < network->nbInternals; ++i )
    {
        network->internals[i] = LAYER_create( network, cfg->internalSize[i], previous );
        previous = network->internals[i];
    }

	// Creation couche de sortie (une seule sortie)
    network->output = LAYER_create( network, cfg->outputSize, previous );

    // Copie des parametres du reseau
    network->learningRate = cfg->learningRate;
    network->lambda = cfg->lambda;
printf( "lambda = %f\n", network->lambda );

    return( network );
}


void NETWORK_applySample( Network* network, Sample* sample )
{
    // Envoi de l'echantillon a la couche d'entree du reseau
    LAYER_applySample( network->input, sample );
}


void NETWORK_destroy( Network* network )
{
    // Si valide
    if( network != NULL )
    {
        // Liberation des couches
        if( network->input ) LAYER_destroy( network->input );
        for( uint16_t i = 0; i < network->nbInternals; ++i )
        {
            if( network->internals[i]) LAYER_destroy( network->internals[i] );
        }
        if( network->output ) LAYER_destroy( network->output );

        // Liberation memoire
        if( network->internals ) free( network->internals );
        free( network );
    }
}

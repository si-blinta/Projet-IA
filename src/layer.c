#include "ia/layer.h"

// System
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

// Local
#include "ia/network.h"


//--- Declaration des fonctions locales ------------------------------------------------------------------------

/** Calcul de la valeur en denominateur de la fonction SOFTMAX
 *
 */
static double softmaxDenominator( Layer* layer, uint32_t nbInputs, const double* inputs );

/** Initialisation des gradients d'erreur de la couche en fonction des sorties attendues (echantillon etiquete)
 *
 *  Cette fonction n'est appelee que dans la couche de sortie, a la fin d'une propagation d'un echantillon
 *  qui possede des valeurs de sortie attendues. Elle initialise la retro-propagation des gradients de l'erreur
 *  pour la couche de sortie
 */
static void initError( Layer* layer, const Sample* sample );


//--- Fonctions publiques --------------------------------------------------------------------------------------

Layer* LAYER_create( struct Network* network, uint32_t size, Layer* previous )
{
    // Allocation de la struture de donnees
    Layer* layer= (Layer*)malloc( sizeof( Layer ) );
    memset( layer, 0, sizeof( Layer ) );

    // Nombre d'entrees des neurones de la couche (calcule ci-dessous)
    uint32_t nbInputs = 0;

    // S'il y a une couche precedente
    if( previous )
    {
        // Interconnexion avec la couche precedente
        layer->previous = previous;
        previous->next = layer;

        // Les neurones de la couche ont comme nombre d'entrees la dimension de la couche precedente
        nbInputs = previous->nbNeurons;
    }
    else
    {
        // Sinon, on est sur la couche d'entree, les neurones ont une entree unique
        layer->previous = NULL;
        nbInputs = 1;
    }

	// Creation des neurones de la couche
	layer->nbNeurons = size;
    layer->neurons = (Neuron**)malloc( size * sizeof( Neuron*) );
    for( uint32_t i = 0; i < size; ++i )
    {
        layer->neurons[i] = NEURON_create( network, nbInputs, i );
    }

    // Creation des valeurs de sortie de la couche
    layer->output = (double*)malloc( layer->nbNeurons * sizeof( double ) );
    memset( layer->output, 0, layer->nbNeurons * sizeof( double ) );

    // Creation des gradients d'erreur de la couche
    layer->error = (double*)malloc( layer->nbNeurons * sizeof( double ) );
    memset( layer->error, 0, layer->nbNeurons * sizeof( double ) );

    return( layer );
}


void LAYER_applySample( Layer* layer, Sample* sample )
{
    // Couche d'entree uniquement
    assert( layer->previous == NULL && "Envoi d'un echantillon sur un couche interne ou de sortie !" );

    // Les donnees en entree doivent avoir la meme dimension que la couche
    assert( sample->inputSize == layer->nbNeurons );

//printf( "--- Layer %u ---------------------------------------------------------\n", layer->nbNeurons );
    // Pour chaque neurone de la couche
    for( uint32_t i = 0; i < layer->nbNeurons; ++i )
    {
        // Transmission de la valeur d'entree (unique) au neurone, et stockage de la valeur de sortie resultante
        const double input = sample->input[i];
        layer->output[i] = NEURON_forward( layer->neurons[i], 1, &input, 0.0 );
    }
//printf( "\n--- FIN Layer %u ---------------------------------------------------------\n", layer->nbNeurons );

    // Propagation des valeurs de sortie (calculees ci-dessus) à la couche suivante
    assert( layer->next && "Configuration de reseau incorrecte (une seule couche) !" );
    LAYER_forward( layer->next, layer->nbNeurons, layer->output, sample );
}


void LAYER_forward( Layer* layer, uint32_t nbInputs, const double* inputs, Sample* sample )
{
    // Si on est sur la couche de sortie, on utilise SOFTMAX comme fonction d'activation
    double denominator = 0.0;
    if( layer->next == NULL )
    {
        // Calcul du denominateur de la fonction SOFTMAX
        denominator = softmaxDenominator( layer, nbInputs, inputs );
    }

//printf( "--- Layer %u ---------------------------------------------------------\n", layer->nbNeurons );
    // Pour chaque neurone de la couche...
    for( uint32_t i = 0; i < layer->nbNeurons; ++i )
    {
        // Propagation des valeurs fournies au neurone, et stockage de la valeur de sortie resultante
        layer->output[i] = NEURON_forward( layer->neurons[i], nbInputs, inputs, denominator );
    }
//printf( "\n--- FIN Layer %u ---------------------------------------------------------\n", layer->nbNeurons );

    // Si couche suivante
    if( layer->next )
    {
        // Propagation des valeurs de sortie (calculees ci-dessus) à la couche suivante
        LAYER_forward( layer->next, layer->nbNeurons, layer->output, sample );
    }
    else
    {
        // On est sur la couche de sortie. Si l'echantillon a des valeurs de sortie
        if( sample->output != NULL )
        {
            // On est en phase d'apprentissage. On calcule les gradients d'erreur en fonction
            // des valeurs de sorties obtenues et celles attendues (disponibles dans l'echantillon)
            initError( layer, sample );
            
            // On lance la retro-propagation vers la couche precedente
            LAYER_backward( layer->previous );
        }
        else
        {
            // On est en phase d'exploitation, on copie les valeurs de sorties obtenues dans l'echantillon
            SAMPLE_setOutput( sample, layer->nbNeurons, layer->output );
        }
    }
}


void LAYER_backward( Layer* layer )
{
    // Si on est sur la couche d'entree, la retro-propagation est terminee
    if( layer->previous == NULL )
    {
        // On met alors a jour les poids dans toutes les couches du reseau (la couche d'entree n'a pas
        // de poids, et ne fait que transmettre directement la valeur en entree)
        LAYER_updateWeights( layer->next );
    }

    // Sinon, on continue la retro-propagation
    else
    {
        // Pour chaque neurone
        for( uint32_t i = 0; i < layer->nbNeurons; ++i )
        {
            // Calcul du gradient d'erreur du neurone en fonction des erreurs remontees par la couche suivante
            layer->error[i] = NEURON_backward( layer->neurons[i], layer->next );
        }

        // On transmet la retro-propagation a la couche precedente
        LAYER_backward( layer->previous );
    }
}


void LAYER_updateWeights( Layer* layer )
{
    // Pour chaque neurone de la couche
    for( uint32_t i = 0; i < layer->nbNeurons; ++i )
    {
        // Mise a jour des poids du neurone
        NEURON_updateWeights( layer->neurons[i], layer->error[i], layer->previous );
    }

    // Si il y a une couche suivante, on propage la mise a jour. Sinon, la mise a jour est terminee
    if( layer->next != NULL ) LAYER_updateWeights( layer->next );
}


void LAYER_destroy( Layer* layer )
{
    // Si valide
    if( layer != NULL )
    {
        // Liberation des neurones
        for( uint32_t i = 0; i < layer->nbNeurons; ++i )
        {
            if( layer->neurons[i] ) NEURON_destroy( layer->neurons[i] );
        }

        // Liberation memoire
        if( layer->neurons ) free( layer->neurons );
        if( layer->output ) free( layer->output );
        if( layer->error ) free( layer->error );
        free( layer );
    }
}

//--- Fonctions locales ----------------------------------------------------------------------------------------

static double softmaxDenominator( Layer* layer, uint32_t nbInputs, const double* inputs )
{
    // Calcul de la somme des exponentielles des entrees ponderees par les poids de chaque neurone
    double denominator = 0.0;
    for( uint32_t i = 0; i < layer->nbNeurons; ++i )
    {
        denominator += exp( NEURON_weightedSum( layer->neurons[i], nbInputs, inputs ) );
    }

    return( denominator );
}

static void initError( Layer* layer, const Sample* sample )
{
    // Les dimensions des sorties obtenues et attendues doivent etre identiques
    assert( layer->nbNeurons == sample->outputSize &&
            "Nombre de valeurs incoherent en sortie d'un echantillon !" );

    // Pour chaque neurone
    for( uint32_t i = 0; i < layer->nbNeurons; ++i )
    {
        // Calcul de l'erreur en sortie (difference entre la valeur obtenue et attendue)
        const double outputError = layer->output[i] - sample->output[i];
        if( sample->digit == i )
        {
            fprintf( stdout, "  INFO - Erreur sur sortie %u = %.6f\n", i, outputError );
        }

        // Initialisation de l'erreur du neurone en fonction de l'erreur en sortie
        layer->error[i] = NEURON_initError( layer->neurons[i], outputError );
    }
}

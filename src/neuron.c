#include "ia/neuron.h"

// System
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

// Local
#include "ia/network.h"
#include "ia/layer.h"

// Pour comparaison de valeurs flottantes avec zero
static const double EPSILON = 0.00000001;


//--- Declaration des fonctions locales ------------------------------------------------------------------------

/** Initialise les poids des neurones dans l'intervalle -1.0..1.0 avec la methode de Xavier/Glorot
 *
 */
static void initWeights( Neuron* neuron );

/** Fonction sigmoide de parametre lambda (fonction d'activation des neurones des couches internes)
 *
 */
static double sigmoid( double lambda, double value );

/** Fonction SOFTMAX (fonction d'activation des neurones de la couche de sortie)
 *
 */
static double softmax( double value, double denominator );


//--- Fonctions publiques --------------------------------------------------------------------------------------

Neuron* NEURON_create( struct Network* network, uint32_t nbInputs, uint32_t index )
{
    // Allocation de la struture de donnees
    Neuron* neuron= (Neuron*)malloc( sizeof( Neuron ) );
    memset( neuron, 0, sizeof( Neuron ) );

    // Copie de l'index et du reseau
    neuron->index = index;
    neuron->network = network;

	// Creation des poids
	neuron->nbInputs = nbInputs;
    neuron->weights = (double*)malloc( nbInputs * sizeof( double ) );;
    memset( neuron->weights, 0, nbInputs * sizeof( double ) );

    // Si une seule entree
    if( neuron->nbInputs == 1 )
    {
        // Le neurone ne fait que transmettre l'entree, le poids (unique) est initialise a 1.0
        neuron->weights[0] = 1.0;
    }
    else
    {
        // Initialisation des poids selon la methode Xavier/Glorot
        initWeights( neuron );
    }

    // Initialisation du biais
    // TODO : methode d'initialisation du biais ?
    neuron->bias = 0.0;

    return( neuron );
}


double NEURON_weightedSum( Neuron* neuron, uint32_t nbInputs, const double* inputs )
{
    // On fait la somme ponderee des entrees
    double sum = 0.0;
    for( uint32_t i = 0; i < nbInputs; ++i ) 
    {
        sum += inputs[i] * neuron->weights[i];
    }

    // On rajoute le biais
    sum += neuron->bias;

    return( sum );
}


double NEURON_forward( Neuron* neuron, uint32_t nbInputs, const double* inputs, double denominator )
{
    // Les dimensions doivent etre identiques
    assert( nbInputs == neuron->nbInputs && "Nombre de valeurs incoherent en entree d'un neurone !" );

    // Si une seule entree, on est dans la couche d'entree, on propage simplement la valeur
    if( nbInputs == 1 )
    {
        neuron->output = *inputs;
//printf( "%f, ", neuron->output );
    }
    else
    {
        // Sinon, on calcule la somme ponderee des entrees
        const double weightedInput = NEURON_weightedSum( neuron, nbInputs, inputs );

        // On passe le resultat a la fonction d'activation : si le denominateur specifie est nul...
        if( fabs( denominator ) < EPSILON )
        {
            // On est dans une couche interne, on applique la fonction sigmoide
            neuron->output = sigmoid( neuron->network->lambda, weightedInput );
        }
        else
        {
            // On est dans la couche de sortie, on applique la fonction SOFTMAX
            neuron->output = softmax( weightedInput, denominator );
        }
//printf( "%f/%f, ", weightedInput, neuron->output );
    }

    return( neuron->output );
}


double NEURON_initError( Neuron* neuron, double outputError )
{
    // Cette fonction n'est appelee que sur la couche de sortie, pour initialiser la retropropagation
    // du gradient d'erreur. La fonction d'activation est donc SOFTMAX. On calcule donc la derivee de
    // SOFTMAX pour la derniere sortie propagee, sachant que :
    // 
    //          SOFTMAX'( x ) = SOFTMAX( x ) * ( 1 - SOFTMAX( x ) )
    //
    const double derivative = neuron->output * ( 1.0 - neuron->output );

    // L'erreur du neurone (couche  de sortie) est le produit de cette derivee avec l'erreur en sortie
    return( derivative * outputError );
}


double NEURON_backward( Neuron* neuron, Layer* nextLayer )
{
    // Somme des erreurs de la couche suivante ponderees par les poids des liens
    // entre ce neurone et chacun des neurones de la couche suivante
    double weightedError = 0.0;
    for( uint32_t i = 0; i < nextLayer->nbNeurons; ++i )
    {
        weightedError += nextLayer->error[i] * nextLayer->neurons[i]->weights[neuron->index];
    }

    // Calcul de la derivee de la fonction sigmoide pour la derniere somme ponderee propagee vers l'avant
    // NOTE: on est forcement sur une couche interne, car la retropropagation est initialisee par la couche
    //       de sortie qui appelle la fonction NEURON_initError(). La fonction d'activation utilisee ici
    //       est donc forcement la fonction sigmoide
    const double derivative = neuron->network->lambda * neuron->output * ( 1.0 - neuron->output );

    // L'erreur du neurone est le produit de cette derivee avec la somme des erreurs ponderees
    return( weightedError * derivative );
}


void NEURON_updateWeights( Neuron* neuron, double error, Layer* previousLayer )
{
    // Pour chaque poids du neurone
    for( uint32_t i = 0; i < neuron->nbInputs; ++i )
    {
        // On ajuste le poids avec le produit des valeurs suivantes :
        // - Taux d'apprentissage du reseau
        // - Erreur du neurone
        // - Valeur en entree du neurone (lors de la derniere propagation) a laquelle s'applique le poids
        // Cette derniere valeur est donc une sortie de la couche precedente (celle qui correspond au poids)
        neuron->weights[i] -= neuron->network->learningRate * error * previousLayer->output[i];
    }
}


void NEURON_destroy( Neuron* neuron )
{
    // Si valide
    if( neuron != NULL )
    {
        // Liberation memoire
        if( neuron->weights ) free( neuron->weights );
        free( neuron );
    }
}

//--- Fonctions locales ----------------------------------------------------------------------------------------

static void initWeights( Neuron* neuron )
{
    // Variance (sur l'intervalle -1.0..1.0) et ecart type calcules a partir du nombre d'entrees du neurone
    double variance = 2.0 / neuron->nbInputs;
    double deviation = sqrt( variance );

    // Pour chaque poids
    for( uint32_t i = 0; i < neuron->nbInputs; ++i )
    {
        // Le poids est initialise en multipliant l'ecart type avec un nombre aleatoire entre -1 et 1
        neuron->weights[i] = deviation * ( ( (double)rand() / RAND_MAX ) * 2.0 - 1.0 );
    }
}


static double sigmoid( double lambda, double value )
{
    // Application de la fonction : x --> 1 / ( 1 + exp( -lambda * x ) )
    return( 1.0 / ( 1.0 + exp( - lambda * value ) ) );

}


static double softmax( double value, double denominator )
{
    // SOFTMAX( Xi ) = exp( Xi ) / SOMME( exp( Xj ) )
    return( exp( value ) / denominator );
}

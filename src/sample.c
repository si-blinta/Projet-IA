#include "ia/sample.h"

// System
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Dimensions des images
#define IMAGE_WIDTH 28
#define IMAGE_HEIGHT 28


//--- Declaration des fonctions locales ------------------------------------------------------------------------

/** Charge l'image specifiee et initialise les entrees de l'echantillon
 *
 */
static int loadImage( Sample* sample, const char* imageFile );


//--- Fonctions publiques --------------------------------------------------------------------------------------

Sample* SAMPLE_create( const char* imageFile, int16_t digit )
{
    // Allocation de la struture de donnees
    Sample* sample= (Sample*)malloc( sizeof( Sample ) );
    memset( sample, 0, sizeof( Sample ) );

	// Chargement de l'image et initialisation des entrees
	if( loadImage( sample, imageFile ) != 0 )
    {
        free( sample );
        return( NULL );
    }

    // Si un chiffre entre 0 et 9 est specifie
    if( digit >= 0 && digit <= 9 )
    {
        // Copie du chiffre
        sample->digit = digit;

        // Initialisation des sorties attendues pour l'echantillon
        const int outputSize = 10;
        sample->outputSize = outputSize;
        sample->output = (double*)malloc( outputSize * sizeof( double ) );
        for( uint16_t i = 0; i < outputSize; ++i )
        {
            // La probabilite est a 1 pour le chiffre fourni et 0 pour les autres
            sample->output[i] = ( i == digit ? 1.0 : 0.0 );
        }
    }
    else
    {
        // Pas d'etiquette (echantillon de test)
        sample->digit = -1;
    }

    return( sample );
}


void SAMPLE_setOutput( Sample* sample, uint32_t nbValues, const double* values )
{
    // Allocation memoire
    sample->outputSize = nbValues;
    sample->output = (double*)malloc( nbValues * sizeof( double ) );

    // Calcul de la somme de valeurs
    double sum = 0.0;
    for( uint16_t i = 0; i < nbValues; ++i ) sum += values[i];

    // Stockage des valeurs normalisees (entre 0.0 et 1.0)
    for( uint16_t i = 0; i < nbValues; ++i )
    {
        sample->output[i] = values[i] / sum;
    }
}


void SAMPLE_destroy( Sample* sample )
{
    // Si valide
    if( sample != NULL )
    {
        // Liberation memoire
        if( sample->input ) free( sample->input );
        if( sample->output ) free( sample->output );
        free( sample );
    }
}

//--- Fonctions locales ----------------------------------------------------------------------------------------

static int loadImage( Sample* sample, const char* imageFile )
{
    // Ouverture du fichier image
    FILE* file = fopen( imageFile, "rb" );
    if( file == NULL )
    {
        fprintf( stderr, "ERREUR - Impossible d'ouvrir le fichier image : %s\n", imageFile );
        return( 1 );
    }

    // Lecture en-tete
    char magic[3];
    int width = 0, height = 0, maxValue = 0;
    fscanf( file, "%2s\n", magic );
    fscanf( file, "%d %d\n", &width, &height );
    fscanf( file, "%d\n", &maxValue );

    // Verification dimensions image
    if( width != IMAGE_WIDTH || height!= IMAGE_HEIGHT )
    {
        fprintf( stderr, "ERREUR - Dimensions d'image incorrectes: %dx%d\n", width, height );
        return( 2 );
    }

    // Allocation memoire pour les entrees de l'echantillon
    sample->inputSize = (uint32_t)( width * height );
    sample->input = (double*)malloc( sample->inputSize * sizeof( double ) );

    // Lecture des pixels
    unsigned char buff[784];
    if( fread( buff, 784, 1, file ) != 1 )
    {
        fprintf( stderr, "ERREUR - Echec lecture fichier image: %s\n", imageFile );
        return( 3 );
    }

    // Normalisation et stockage de pixels comme entrees de l'echantillon
    for( uint32_t i = 0; i < sample->inputSize; ++i )
    {
        sample->input[i] = (double)buff[i] / (double)maxValue;
    }

    // Fermeture du fichier
    fclose( file );

    return( 0 );
}

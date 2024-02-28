#include "ia/config.h"

// System
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>

// Taille de buffer
#define BUFF_SIZE 256


//--- Declaration fonctions locales ----------------------------------------------------------------------------

/** Extraction du couple cle/valeur de la ligne de configuration specifiee
 *
 */
static int extractConfigValue( Config* config, const char* line );


//--- Fonctions publiques --------------------------------------------------------------------------------------

Config* CONFIG_create()
{
    // Allocation de la struture de donnees
    Config* config= (Config*)malloc( sizeof( Config ) );
    memset( config, 0, sizeof( Config ) );

    return( config );
}


int CONFIG_readFromFile( Config* config, const char* fileName )
{
    // Ouverture du fichier
    FILE* cfgFile = fopen( fileName, "r" );
    if( cfgFile == NULL )
    {
        fprintf( stdout, "ERREUR - Impossible d'ouvrir le fichier : %s\n", fileName );
        return( 1 );
    }

    // Lecture du fichier
    while( 1 )
    {
        // Lecture de la ligne courante
        char line[BUFF_SIZE];
        if( fgets( line, BUFF_SIZE, cfgFile ) == NULL )
        {
            if( ferror( cfgFile ) !=0 )
            {
                // Erreur de lecture
                fprintf( stderr, "ERREUR - Echec de lecture de la configuration : %s\n", strerror( errno ) );
                return( 2 );
            }
            else
            {
                // Fin de fichier
                break;
            }
        }

        // Suppression caractere <newline>
        const size_t length = strlen( line );
        if( line[length - 1] == '\n' ) line[length - 1] = '\0';

        // Extraction du mot cle et de la valeur, et mise a jour de la configuration
        const int status = extractConfigValue( config, line );
        if( status != 0 )
        {
            fprintf( stderr, "Format de configuration incorrect [erreur = %d, ligne = %s]\n", status, line );
            fclose( cfgFile );
            return( 3 );
        }
    }

    // Fermeture du fichier
    fclose( cfgFile );

    return( 0 );
}


void CONFIG_destroy( Config* config )
{
    // Si config valide
    if( config != NULL )
    {
        // Liberation memoire
        free( config );
    }
}

//--- Fonctions locales ----------------------------------------------------------------------------------------

static int extractConfigValue( Config* config, const char* line )
{
    // Extraction du mot-cle
    char key[32];
    char* ptr = strtok( (char*)line, " \t" );
    if( ptr == NULL ) return( 1 );
    strcpy( key, ptr );

    // Suppression ':' final
    key[strlen( key ) - 1] = '\0';

    // Extraction de la valeur
    ptr = strtok( NULL, " \t" );
    if( ptr == NULL ) return( 2 );

    // Conversion de la valeur en reel
    char* endptr = ptr;
    const double value = strtod( ptr, &endptr );
    if( *endptr != '\0' ) return( 3 );

    // Verification fin de ligne
    if( strtok( NULL, " \t" ) != NULL ) return( 4 );

    // En fonction du mot-cle
    if( strcmp( key, "input" ) == 0 )
    {
        // Dimension de la couche d'entree
        config->inputSize = (uint32_t)value;
    }
    else if( strcmp( key, "output" ) == 0 )
    {
        // Dimension de la couche de sortie
        config->outputSize = (uint32_t)value;
    }
    else if( strcmp( key, "internal" ) == 0 )
    {
        // Dimension d'une (nouvelle) couche interne
        config->internalSize[ config->nbLayers++ ] = (uint32_t)value;
    }
    else if( strcmp( key, "rate" ) == 0 )
    {
        // Taux d'apprentissage
        config->learningRate = value;
    }
    else if( strcmp( key, "lambda" ) == 0 )
    {
        // Parametre lambda de la sigmoide
        config->lambda = value;
    }
    else
    {
        // Mot-cle inconnu
        return( 5 );
    }

    return( 0 );
}

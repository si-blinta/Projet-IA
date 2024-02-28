// System
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>

// Local
#include "ia/network.h"
#include "ia/config.h"
#include "ia/sample.h"

// Repertoires des images d'entrainement et de test
static const char* DIR_TRAINING = "data/images/training";
static const char* DIR_TESTING = "data/images/testing";


//--- Declaration fonctions locales ----------------------------------------------------------------------------

/** Phase d'apprentissage
 *
 */
static int learning( Network* network, const char* imageFolder );

/** Phase d'exploitation
 *
 */
static void testing( Network* network, const char* imageFolder );

/** Construit un nom de fichier image complet, et retourne l'etiquette (chiffre) associee
 *
 */
static int16_t buildPath( char* filePath, const char* dirName, const char* fileName );



//--- main() ----------------------------------------------------------------------------------------------------------

int main( int argc, char* argv[] )
{
    // Verification ligne de commande
    if( argc != 2 )
    {
        fprintf( stderr, "Ligne de commande incorrecte!\n" );
        fprintf( stderr, "Usage: reseau CONFIG\n" );
        return( 1 );
    }
    
    // Lecture de la configuration
    Config* cfg = CONFIG_create();
    if( CONFIG_readFromFile( cfg, argv[1] ) != 0 ) return( 2 );

    // Creation du reseau
    Network* network = NETWORK_create( cfg );

    // Phase d'apprentissage
    printf( "--- DEBUT PHASE D'APPRENTISSAGE --------------------------------------------------------\n" );
    if( learning( network, DIR_TRAINING ) != 0 ) return( 3 );
    printf( "--- FIN PHASE D'APPRENTISSAGE   --------------------------------------------------------\n" );

    // Phase d'exploitation
    printf( "--- DEBUT PHASE DE TEST ----------------------------------------------------------------\n" );
    testing( network, DIR_TESTING );
    printf( "--- FIN PHASE DE TEST ------------------------------------------------------------------\n" );

    // Liberation memoire
    NETWORK_destroy( network );
    CONFIG_destroy( cfg );

    return( 0 );
}


//--- Fonctions locales ----------------------------------------------------------------------------------------

static int learning( Network* network, const char* imageFolder )
{
    // Ouverture du repertoire qui contient les images pour la phase d'apprentissage
    DIR* dir = opendir( imageFolder );
    if( dir == NULL )
    {
        fprintf( stderr, "ERREUR - Impossible d'ouvror le répertoire %s\n", imageFolder );
        return( 1 );
    }

    // Pour chaque entree du repertoire
    uint32_t nbImages = 0;
    struct dirent *entry = NULL;
    // Compteur pour limiter l'entrainement
    int cpt = 0;    
    while( ( entry = readdir( dir ) ) != NULL && cpt <60000 )
    {
        // On ignore tout fichier qui ne commence pas par "image-"
        const char* fileName = entry->d_name;
        if( strncmp( fileName, "image-", 6 ) != 0 ) continue;

        // On construit le chemin complet, et on recupere le chiffre associe a l'image
        char filePath[256];
        const int16_t digit = buildPath( filePath, imageFolder, fileName );
        if( digit < 0 || digit > 9 )
        {
            fprintf( stderr, "ERREUR - Etiquette d'image incorrecte: %s\n", fileName );
            continue;
        }

        // Creation d'un echantillon sur la base de l'image
        fprintf( stdout, "> Apprentissage (step #%u) avec %s [chiffre = %d]...\n", ++nbImages, fileName, digit );
        Sample* sample = SAMPLE_create( filePath, digit );
        if( sample == NULL ) return( 1 );
        NETWORK_applySample( network, sample );
        fprintf( stdout, "< OK\n" );
        cpt++;
    }

    // Fermeture du repertoire
    closedir( dir );

    return( 0 );
}

static void testing( Network* network, const char* imageFolder )
{
    // Ouverture du repertoire qui contient les images pour la phase d'apprentissage
    DIR* dir = opendir( imageFolder );
    if( dir == NULL )
    {
        fprintf( stderr, "ERREUR - Impossible d'ouvror le répertoire %s\n", imageFolder );
        return;
    }

    // Pour chaque entree du repertoire
    uint32_t nbImages = 0;
    uint32_t nb_ImagesValides = 0;
    struct dirent *entry = NULL;
    while( ( entry = readdir( dir ) ) != NULL )
    {
        // On ignore tout fichier qui ne commence pas par "image-"
        const char* fileName = entry->d_name;
        if( strncmp( fileName, "image-", 6 ) != 0 ) continue;

        // On construit le chemin complet, et on recupere le chiffre associe a l'image
        char filePath[256];
        const int16_t digit = buildPath( filePath, imageFolder, fileName );
        if( digit < 0 || digit > 9 )
        {
            fprintf( stderr, "ERREUR - Etiquette d'image incorrecte: %s\n", fileName );
            continue;
        }

        // Creation d'un echantillon sur la base de l'image, mais sans les sorties attendues
        fprintf( stdout, "> Phase de test (step #%u) avec %s [chiffre = %d]...\n", ++nbImages, fileName, digit );
        Sample* sample = SAMPLE_create( filePath, -1 );
        if( sample != NULL )
        {
            // Application de l'echantillon
            NETWORK_applySample( network, sample );

            // Recherche de la probabilite la plus elevee
            double maxProba = 0.0;
            int16_t maxIndex = 0;
            for( int16_t i = 0; i < sample->outputSize; ++i )
            {
                if( sample->output[i] > maxProba )
                {
                    maxProba = sample->output[i];
                    maxIndex = i;
                }
            }

            // Si la probabilite max correspond au chiffre, le test est concluant (et faux sinon)
            if( maxIndex == digit ){
                fprintf( stdout, "< OK (chiffre identifié avec une probabilité de %f)\n", maxProba );
                nb_ImagesValides++;
            }
                
            else
                fprintf( stdout, "< KO (chiffre identifié = %d, chiffre attendu = %d)\n", maxIndex, digit );
        }
        else
        {
            fprintf( stdout, "< ERROR\n" );
        }

    }
    printf("INFO Precision = %lf\n",((double)nb_ImagesValides/(double)nbImages)*100);
}


static int16_t buildPath( char* filePath, const char* dirName, const char* fileName )
{
    // Constrution du chemin complet
    sprintf( filePath, "%s/%s", dirName, fileName );

    // Extraction du chiffre associe a l'image du nom du fichier. Le fichier suit le format
    // "image-<num>-label-<digit>.pgm". On veut extraire le segment entre le dernier '-' et le '.'
    char* ptrDash = strrchr( fileName, '-' );
    char* ptrDot = strchr( fileName, '.' );
    if( ptrDot - ptrDash != 2 )
    {
        fprintf( stderr, "ERREUR - Format du nom de fichier image incorrect: %s\n", fileName );
        return( -1 );
    }
    char digit[2];
    digit[0] = *( ++ptrDash );
    digit[1] = '\0';

    return( (int16_t)atoi( digit ) );
}

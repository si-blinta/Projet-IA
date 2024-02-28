#ifndef _IA_CONFIG_H_
#define _IA_CONFIG_H_

// System
#include <stdint.h>


//--------------------------------------------------------------------------------------------------------------
// Module: CONFIG
// Description:
//      Configuration d'un reseau de neurones
//--------------------------------------------------------------------------------------------------------------

// Nombre max de couches internes
#define MAX_INTERNALS 32

/** Structure de donnees associee la configuration
 *
 */
typedef struct
{
    uint32_t inputSize;                     // Dimension de la couche d'entree
    uint16_t nbLayers;                      // Nombre de couches internes
    uint32_t internalSize[MAX_INTERNALS];   // Dimensions des couches internes
    uint32_t outputSize;                    // Dimension de la couche de sortie
    double learningRate;                    // Taux d'appentissage du reseau
    double lambda;                          // Parametre lambda des fonctionis sigmoides des neurones
} Config;


/** Creation de la configuration
 *
 */
extern Config* CONFIG_create();

/** Lecture de la configuration a partir d'un fichier
 *
 */
extern int CONFIG_readFromFile( Config* config, const char* fileName );

/** Destruction de la configuration
 *
 */
extern void CONFIG_destroy( Config* config );

#endif // _IA_CONFIG_H_

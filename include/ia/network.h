#ifndef _IA_NETWORK_H_
#define _IA_NETWORK_H_

// System
#include <stdint.h>

// Local
#include "ia/layer.h"
#include "ia/config.h"
#include "ia/sample.h"


//--------------------------------------------------------------------------------------------------------------
// Module: NETWORK
// Description:
//--------------------------------------------------------------------------------------------------------------

/** Structure de donnees associee a un reseau de neurones
 *
 */
typedef struct Network
{
    Layer* input;                   // Couche d'entree
    uint16_t nbInternals;           // Nombre de couches internes
    Layer** internals;              // Couches internes
    Layer* output;                  // Couche de sortie
    double learningRate;            // Taux d'appentissage du reseau
    double lambda;                  // Parametre lambda des fonctionis sigmoides des neurones
} Network;


/** Creation d'un reseau de neurones (a partir de la configuration)
 *
 */
extern Network* NETWORK_create( const Config* cfg );

/** Envoi d'un echantillon en entree du reseau
 *
 *  Si l'echantillon possede des valeurs de sortie, alors il s'agit d'une phase d'apprentissage. Dans le
 *  cas contraire, il s'agit d'une phase d'exploitation, et les valeurs de sortie sont copiees dans
 *  l'echantillon en sortie du reseau
 */
extern void NETWORK_applySample( Network* network, Sample* sample );

/** Destruction d'un reseau
 *
 */
extern void NETWORK_destroy( Network* network );

#endif // _IA_NETWORK_H_

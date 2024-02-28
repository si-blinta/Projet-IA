#ifndef _IA_LAYER_H_
#define _IA_LAYER_H_

// System
#include <stdint.h>

// Local
#include "ia/neuron.h"
#include "ia/sample.h"


//--------------------------------------------------------------------------------------------------------------
// Module: LAYER
// Description:
//      Modelisation d'un couche de neurones du reseau
//--------------------------------------------------------------------------------------------------------------

// Pre-declarations
struct Network;

/** Structure de donnees associee a une couche
 *
 */
typedef struct Layer
{
    uint32_t nbNeurons;         // Nombre de neurones constituant la couche
    Neuron** neurons;           // Neurones de la couche
    double* output;             // Valeurs de sortie de la couche (une par neurone)
    double* error;              // Gradients de l'erreur lors de la retropropagation (un par neurone)
    struct Layer* previous;     // Couche precedente (si nul, on est dans la couche d'entree)
    struct Layer* next;         // Couche suivante (si nul, on est dans la couche de sortie)
} Layer;


/** Creation d'une couche de reseau
 *
 *  On passe :
 *  - Le reseau auquel appertient la couche
 *  - La taille de la couche
 *  - La couche precedente (si elle existe)
 */
extern Layer* LAYER_create( struct Network* network, uint32_t size, Layer* previous );

/** Envoi d'un echantillon a la couche
 *
 *  Cette fonction n'est appelee que pour la couche d'entree du reseau
 */
extern void LAYER_applySample( Layer* layer, Sample* sample );

/** Propagation des valeurs de sortie provenant de la couche precedente
 *
 */
extern void LAYER_forward( Layer* layer, uint32_t nbInputs, const double* inputs, Sample* sample );

/** Retro-propagation des gradients de l'erreur provenant de la couche suivante
 *
 */
extern void LAYER_backward( Layer* layer );

/** Mise a jour des poids des neurones de la couche, et de la couche suivante
 *
 *  Cette fonction est appelee sur la couche d'entree du reseau, de sorte a mettre a jour l'ensemble des
 *  poids du reseau (pour tous les neurones) a la fin d'une retro-propagation. La mise a jour se fait
 *  alors en fonction des gradients d'erreur qui ont ete calcules lors de cette retro-propagation
 */
extern void LAYER_updateWeights( Layer* layer );

/** Destruction d'une couche
 *
 */
extern void LAYER_destroy( Layer* layer );

#endif // _IA_LAYER_H_

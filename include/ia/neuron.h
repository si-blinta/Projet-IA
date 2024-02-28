#ifndef _IA_NEURON_H_
#define _IA_NEURON_H_

// System
#include <stdint.h>


//--------------------------------------------------------------------------------------------------------------
// Module: NEURON
// Description:
//      Modelisation d'un neurone du reseau
//--------------------------------------------------------------------------------------------------------------

// Pre-declarations
struct Network;
struct Layer;

/** Structure de donnees associee a un neurone
 *
 */
typedef struct Neuron
{
    uint32_t index;             // Index du neurone dans sa couche
    uint32_t nbInputs;          // Nombre d'entrees du neurone (soit la taille de la couche precedente)
    double* weights;            // Les poids qui ponderent les entrees
    double bias;                // Biais du neurone
    double output;              // Derniere sortie calculee (lors de la phase de propagation)
    struct Network* network;    // Reseau auquel appartient le neurone
} Neuron;


/** Creation d'un neurone
 *
 *  On fournit :
 *  - Le reseau auquel appartient le neurone
 *  - Le nombre d'entrees du neurone (dimension de la couche precedente
 *  - L'index du neurone dans sa couche
 */
extern Neuron* NEURON_create( struct Network* network, uint32_t nbInputs, uint32_t index );

/** Calcul de la somme des entrees specifiees ponderees par les poids du neurone
 *
 */
extern double NEURON_weightedSum( Neuron* neuron, uint32_t nbInputs, const double* inputs );

/** Propagation des valeurs de sortie provenant de la couche precedente (ou d'une valeur d'entree du reseau)
 *
 *  La fonction applique la fonction de transfert du neurone aux valeurs en entrees, et retourne le resultat.
 *  Le denominateur n'est utilise que par les neurones de la couche de sortie, qui utilise une fonction
 *  d'activation SOFTMAX (contrairement aux couches internes qui utilise la fonction sigmoide)
 */
extern double NEURON_forward( Neuron* neuron, uint32_t nbInputs, const double* inputs, double denominator );

/** Initialise l'erreur du neurone a partir de l'ecart entre la sortie attendue et celle obtenue
 *
 *  Cette fonction est utilisee pour les neurones de la couche de sortie, au demarrage de la
 *  retropropagation des gradients d'erreur (pour les autres couches, on utilise NEURON_backward())
 */
extern double NEURON_initError( Neuron* neuron, double outputError );

/** Retro-propagation des gradients d'erreur provenant de la couche suivante
 *
 */
extern double NEURON_backward( Neuron* neuron, struct Layer* nextLayer );

/** Mise a jour des poids du neurone
 *
 *  La fonction est appelee a la fin de chaque retro-propagation, de sorte à ce que les poids du neurone
 *  soient ajustes en fonction de l'erreur calculee (lors de la retro-propagation) et du taux
 *  d'apprentissage du reseau
 */
extern void NEURON_updateWeights( Neuron* neuron, double error, struct Layer* previousLayer );

/** Destruction d'un neurone
 *
 */
extern void NEURON_destroy( Neuron* neuron );

#endif // _IA_NEURON_H_

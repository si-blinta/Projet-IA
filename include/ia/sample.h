#ifndef _IA_SAMPLE_H_
#define _IA_SAMPLE_H_

// System
#include <stdint.h>


//--------------------------------------------------------------------------------------------------------------
// Module: SAMPLE
// Description:
//      Echantillon de donnees pour l'apprentissagedu reseau. Les echantillons sont construits a partir 
//      d'image 28x28 (source http://yann.lecun.com/exdb/mnist/) au format PGM, dont le nom de fichier
//      suit le format "image-<num>-label-<chiffre>.pgm", avec:
//      - <num> numero de l'image (juste pour avoir un nom de fichier unique)
//      - <chiffre> un chifre entre 0 et 9, qui constitue l'etiquette de l'echantillon en phase d'apprentissage
//--------------------------------------------------------------------------------------------------------------

/** Structure de donnees associee a un echantillon
 *
 */
typedef struct
{
    uint32_t inputSize;
    double* input;
    uint32_t outputSize;
    double* output;
    int16_t digit;
} Sample;


/** Creation d'un echantillon a partir d'une image
 *
 *  Si un chiffre compris entre 0 et 9 est fourni, l'echantillon va servir pour l'apprentissage, et alors
 *  on calcule les sorties. Sinon, il va servir pour la phase de test, et les sorties seront celles obtenues
 *  en sortie du reseau
 */
extern Sample* SAMPLE_create( const char* imageFile, int16_t digit );

/** Copie et normalisation (intervalle 0..1 ) des valeurs de sorties
 *
 *  Cette fonction est appelee en phase d'exploitation pour stocker le resultat en sortie du reseau
 *  pour l'echantillon specifie
 */
extern void SAMPLE_setOutput( Sample* sample, uint32_t nbValues, const double* values );

/** Destruction d'un echantillon
 *
 */
extern void SAMPLE_destroy( Sample* sample );

#endif // _IA_SAMPLE_H_

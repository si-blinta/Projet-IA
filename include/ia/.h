:1,$s/NAME/xxx/g
:1,$s/Name/xxx/g
:1,$s/name/xxx/g
#ifndef _IA_NAME_H_
#define _IA_NAME_H_


//--------------------------------------------------------------------------------------------------------------
// Module: NAME
// Description:
//--------------------------------------------------------------------------------------------------------------

/** Structure de donnees associee a...
 *
 */
typedef struct
{
} Name;


/** Creation d'un...
 *
 */
extern Name* NAME_create();

/** Destruction d'un...
 *
 */
extern void NAME_destroy( Name* name );

#endif // _IA_NAME_H_

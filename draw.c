#include "draw.h"
#include <stdio.h>
#include <stdlib.h>

/*
* Ajoute un sommet à une liste de sommets.
*/
struct sommet* sommet_add(struct sommet* list, int x, int y)
{
	// Allocation
	struct sommet* new = malloc(sizeof(struct sommet));
	
	new->next = NULL;
	new->x = x;
	new->y = y;
	
	if(list != NULL)
	{
		// Ajout en fin de liste
		struct sommet* cursor = list;
		while(cursor->next != NULL) cursor = cursor->next;
		
		cursor->next = new;
	}
	else
	{
		// Je deviens la nouvelle tête de liste
		list = new;
	}
	
	return list;
}

/*
* Récupère le dernier sommet d'une liste.
*/
struct sommet* sommet_get_last(struct sommet* list)
{
	if(list != NULL)
	{
		struct sommet* cursor = list;
		while(cursor->next != NULL) cursor = cursor->next;
		
		return cursor;
	}
	else
	{
		return NULL;
	}
}

/*
* Supprime un sommet de la liste.
*/
struct sommet* sommet_remove(struct sommet* list, struct sommet* victim)
{
	struct sommet* cursor = list;
	
	if(cursor == victim) // Je suis le 1er élément de la liste
	{
		/*
		* Le successeur de la victime devient la nouvelle tête de liste.
		*/
		
		struct sommet* newHead = cursor->next;
		free(victim);
		return newHead;
	}
	else
	{
		while(cursor != NULL)
		{
			if(cursor->next == victim)
			{
				/*
				* La chaîne a->b->c devient a->c et on peut supprimer b.
				*/

				cursor->next = cursor->next->next; // tordu, n'est-ce pas ?
				free(victim);
				
				cursor = NULL; // sortie de boucle
			}
		}
		
		// La tête de liste n'a pas changé !
		return list;
	}
}

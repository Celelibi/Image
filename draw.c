#include "draw.h"
#include <stdio.h>
#include <stdlib.h>

/*
* Ajoute un sommet à une liste de sommets.
*/
sommet* sommet_add(sommet* list, int x, int y)
{
	// Allocation
	sommet* new = malloc(sizeof(sommet));
	
	new->next = NULL;
	new->x = x;
	new->y = y;
	
	if(list != NULL)
	{
		// Ajout en fin de liste
		sommet* cursor = list;
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
sommet* sommet_get_last(sommet* list)
{
	if(list != NULL)
	{
		sommet* cursor = list;
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
sommet* sommet_remove(sommet* list, sommet* victim)
{
	sommet* cursor = list;
	
	if(cursor == victim) // Je suis le 1er élément de la liste
	{
		/*
		* Le successeur de la victime devient la nouvelle tête de liste.
		*/
		
		sommet* newHead = cursor->next;
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

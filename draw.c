#include "draw.h"
#include <stdio.h>
#include <stdlib.h>

/*
* Ajoute un sommet à une liste de sommets.
*/
struct vertex* polygon_append_vertex(struct vertex* list, int x, int y)
{
	// Allocation
	struct vertex* new = malloc(sizeof(struct vertex));
	
	new->x = x;
	new->y = y;
	
	if(list != NULL)
	{
		// Ajout en fin de liste
		struct vertex* cursor = list;
		while(cursor->next != list) cursor = cursor->next;
		
		new->prev = cursor;
		new->next = list;
		cursor->next = new;
		list->prev = new;
	}
	else
	{
		// Je deviens la nouvelle tête de liste
		list = new;
		list->prev = list;
		list->next = list;
		
	}
	
	return list;
}

/*
* Supprime un sommet de la liste.
*/
struct vertex* polygon_remove_vertex(struct vertex* list, struct vertex* victim)
{
	struct vertex* cursor = list;
	
	if(cursor == victim) // Je suis le 1er élément de la liste
	{
		/*
		* Le successeur de la victime devient la nouvelle tête de liste.
		*/
		
		struct vertex* newHead = cursor->next;
		newHead->prev = victim->prev;
		victim->prev->next = newHead;
		free(victim);
		return newHead;
	}
	else
	{
		do
		{
			if(cursor->next == victim)
			{
				/*
				* La chaîne a->b->c devient a->c et on peut supprimer b.
				*/

				cursor->next = cursor->next->next; // tordu, n'est-ce pas ?
				cursor->next->prev = cursor;
				free(victim);
				
				cursor = NULL; // sortie de boucle
			}
		}while(cursor != list);
		
		// La tête de liste n'a pas changé !
		return list;
	}
}

/*
* Récupère le dernier sommet d'une liste.
*/
static struct vertex* polygon_get_last_vertex(struct vertex* list)
{
	if(list != NULL)
	{
		struct vertex* cursor = list;
		while(cursor->next != NULL) cursor = cursor->next;
		
		return cursor;
	}
	else
	{
		return NULL;
	}
}

/*
* Trace une droite de Bresenham entre deux sommets.
*/
void segment_rasterize(Image *img, int xA, int yA, int xB, int yB)
{
	int dx = xB - xA;
	int dy = yB - yA;
	int e; // erreur
	int x = xA;
	int y = yA;

	if (dx != 0)
	{
		if (dx > 0)
		{
			if (dy != 0)
			{
				if (dy > 0)
				{
					// vecteur oblique dans le 1er quadran
					if (dx >= dy)
					{
						// vecteur diagonal ou oblique proche de l'horizontale, dans le 1er octant
						e = dx; // e est positif
						dx = 2 * e;
						dy = 2 * dy;
						for (x = xA; x < xB; x++)
						{
							I_plot(img, x, y);
							e = e - dy;
							if (e < 0)
							{
								y++;
								e = e + dx;
							}
						}
					}
					else // dx < dy
					{
						// vecteur oblique proche de la verticale, dans le 2nd octant
						e = dy; // e est positif
						dy = 2 * e;
						dx = 2 * dx;
						for (y = yA; y < yB; y++)
						{
							I_plot(img, x, y);
							e = e - dx;
							if (e < 0)
							{
								x++;
								e = e + dy;
							}
						}
					}
				}
				else // dy < 0
				{
					// vecteur oblique dans le 4e cadran
					if (dx >= -dy)
					{
						// vecteur diagonal ou oblique proche de l'horizontale, dans le 8e octant
						e = dx; // e est positif
						dx = 2 * e;
						dy = 2 * dy;
						for (x = xA; x < xB; x++)
						{
							I_plot(img, x, y);
							e = e + dy;
							if (e < 0)
							{
								y--;
								e = e + dx;
							}
						}
					}
					else // dx < -dy
					{
						// vecteur oblique proche de la verticale, dans le 7e octant
						e = dy; // e est négatif
						dy = 2 * e;
						dx = 2 * dx;
						for (y = yA; y > yB; y--)
						{
							I_plot(img, x, y);
							e = e + dx;
							if (e > 0)
							{
								x++;
								e = e + dy;
							}
						}
					}
				}
			}
			else // dy = 0
			{
				// vecteur horizontal vers la droite
				for (x = xA; x < xB; x++)
					I_plot(img, x, y);
			}
		}
		else // dx < 0
		{
			if (dy != 0)
			{
				if (dy > 0)
				{
					// vecteur oblique dans le 2nd quadran
					if (-dx >= dy)
					{
						// vecteur diagonal ou oblique proche de l'horizontale, dans le 4e octant
						e = dx; // e est négatif
						dx = 2 * e;
						dy = 2 * dy;
						for (x = xA; x > xB; x--)
						{
							I_plot(img, x, y);
							e = e + dy;
							if (e >= 0)
							{
								y++;
								e = e + dx;
							}
						}
					}
					else // -dx < dy
					{
						// vecteur oblique proche de la verticale, dans le 3e octant
						e = dy; // e est positif
						dy = 2 * e;
						dx = 2 * dx;
						for (y = yA; y < yB; y++)
						{
							I_plot(img, x, y);
							e = e + dx;
							if (e <= 0)
							{
								x--;
								e = e + dy;
							}
						}
					}
				}
				else // dy < 0
				{
					// vecteur oblique dans le 3e cadran
					if (dx <= dy)
					{
						// vecteur diagonal ou oblique proche de l'horizontale, dans le 5e octant
						e = dx; // e est négatif
						dx = 2 * e;
						dy = 2 * dy;
						for (x = xA; x > xB; x--)
						{
							I_plot(img, x, y);
							e = e - dy;
							if (e >= 0)
							{
								y--;
								e = e + dx;
							}
						}
					}
					else // dx > dy
					{
						// vecteur oblique proche de la verticale, dans le 6e octant
						e = dy; // e est négatif
						dy = 2 * e;
						dx = 2 * dx;
						for (y = yA; y > yB; y--)
						{
							I_plot(img, x, y);
							e = e - dx;
							if (e >= 0)
							{
								x--;
								e = e + dy;
							}
						}
					}
				}
			}
			else // dy = 0
			{
				// vecteur horizontal vers la gauche
				for (x = xA; x > xB; x--)
					I_plot(img, x, y);
			}
		}
	}
	else // dx = 0
	{
		if (dy != 0)
		{
			if (dy > 0)
			{
				// vecteur vertical vers le haut
				for (y = yA; y < yB; y++)
					I_plot(img, x, y);
			}
			else // dy < 0
			{
				// vecteur vertical vers le bas
				for (y = yA; y > yB; y--)
					I_plot(img, x, y);
			}
		}
	}
}

void polygon_rasterize(struct polygon *p, Image *img)
{
	if(p->v_list != NULL) // Si on a au moins un sommet de placé...
	{
		struct vertex* cursor = p->v_list;
		while(cursor->next != p->v_list)
		{
			segment_rasterize(img, cursor->x, cursor->y, cursor->next->x, cursor->next->y);

			cursor = cursor->next;
		}

		if(p->is_closed)
		{
			// Relie le dernier élément au premier
			segment_rasterize(img, cursor->x, cursor->y, p->v_list->x, p->v_list->y);
		}
	}
}

void drawing_rasterize(struct drawing *d, Image *img)
{
	Color black = C_new(0,0,0);
	I_fill(img, black);

	while(d->p_list->next != NULL)
	{
		polygon_rasterize(d->p_list, img);
		d->p_list = d->p_list->next;
	}
}


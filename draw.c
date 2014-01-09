#include "draw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
* Ajoute un sommet à une liste de sommets.
*/
struct vertex* polygon_append_vertex(struct polygon* poly, int x, int y)
{
	// Allocation
	struct vertex* new = malloc(sizeof(*new));

	memset(new, 0, sizeof(*new));

	new->x = x;
	new->y = y;

	if(poly->v_list != NULL)
	{
		// Ajout en fin de liste
		new->prev = poly->v_last;
		new->next = poly->v_last->next;
		poly->v_last->next = new;

		if (new->next != NULL)
			new->next->prev = new;

	}
	else
	{
		// Je deviens la nouvelle tête de liste
		// next et prev sont déjà NULL
		poly->v_list = new;
	}

	poly->v_last = new;
	poly->vertexcnt++;

	return new;
}

/*
* Supprime un sommet de la liste.
*/
struct vertex* polygon_remove_vertex(struct polygon* poly, struct vertex* victim)
{
	/*
	 * Le successeur de la victime devient la nouvelle tête de liste.
	 */
	if (poly->v_list == victim)
		poly->v_list = poly->v_list->next;
	if (poly->v_last == victim)
		poly->v_last = victim->prev;

	if (victim->next != NULL)
		victim->next->prev = victim->prev;
	if (victim->prev != NULL)
		victim->prev->next = victim->next;

	free(victim);
	poly->vertexcnt--;

	return poly->v_list;
}

/*
 * Ferme un polygone overt et inversement. Renvoie 1 si le polygone vient d'être fermé, 0 sinon.
 */
int polygon_toggle_close(struct polygon* poly)
{
	if (poly->v_list == NULL)
		return 0;

	// Polygone ouvert -> on le ferme
	if (poly->v_list->prev == NULL)
	{
		poly->v_last->next = poly->v_list;
		poly->v_list->prev = poly->v_last;
		return 1;
	}

	// Polygone fermé -> on l'ouvre
	poly->v_last->next = NULL;
	poly->v_list->prev = NULL;
	poly->is_filled = 0;
	return 0;
}

int polygon_is_closed(struct polygon* poly)
{
	return poly->v_list != NULL && poly->v_list->prev != NULL;
}

/*
 * Ajoute un polygone vide à la fin de la liste et le renvoie
 */
void drawing_new_polygon(struct drawing *d)
{
	struct polygon *p, *last;

	p = malloc(sizeof(*p));
	if (p == NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	memset(p, 0, sizeof(*p));

	if (d->p_list == NULL) {
		d->p_list = p;
	} else {
		last = d->p_list;
		while (last->next != NULL)
			last = last->next;

		last->next = p;
	}

	d->p_active = p;
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
					// vecteur oblique dans le 1er cadran
					if (dx >= dy)
					{
						// vecteur diagonal ou oblique proche de l'horizontale, dans le 1er octant
						e = dx; // e est positif
						dx = 2 * e;
						dy = 2 * dy;
						for (x = xA; x <= xB; x++)
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
						for (y = yA; y <= yB; y++)
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
						for (x = xA; x <= xB; x++)
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
						for (y = yA; y >= yB; y--)
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
				for (x = xA; x <= xB; x++)
					I_plot(img, x, y);
			}
		}
		else // dx < 0
		{
			if (dy != 0)
			{
				if (dy > 0)
				{
					// vecteur oblique dans le 2nd cadran
					if (-dx >= dy)
					{
						// vecteur diagonal ou oblique proche de l'horizontale, dans le 4e octant
						e = dx; // e est négatif
						dx = 2 * e;
						dy = 2 * dy;
						for (x = xA; x >= xB; x--)
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
						for (y = yA; y <= yB; y++)
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
						for (x = xA; x >= xB; x--)
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
						for (y = yA; y >= yB; y--)
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
				for (x = xA; x >= xB; x--)
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
				for (y = yA; y <= yB; y++)
					I_plot(img, x, y);
			}
			else // dy < 0
			{
				// vecteur vertical vers le bas
				for (y = yA; y >= yB; y--)
					I_plot(img, x, y);
			}
		}
	}
}

void polygon_rasterize(struct polygon *p, Image *img)
{
	// Si on a au moins un sommet de placé...
	if(p->v_list != NULL)
	{
		struct vertex* cursor = p->v_list;
		while(cursor->next != NULL && cursor->next != p->v_list)
		{
			segment_rasterize(img, cursor->x, cursor->y, cursor->next->x, cursor->next->y);
			cursor = cursor->next;
		}
		if (cursor->next != NULL)
			segment_rasterize(img, cursor->x, cursor->y, cursor->next->x, cursor->next->y);
	}
}

/* Helper function of polygon_ysorted_vertex, used as argument to qsort */
static int polygon_ysorted_compare(const void *a, const void *b)
{
	struct vertex* const* va = a;
	struct vertex* const* vb = b;

	return (*va)->y - (*vb)->y;
}

/*
 * This function is a helper for polygon_fill
 * It build an array of pointers to the vertex of the polygon.
 * This array is ordered by y-coordinate.
 * This function will fill the memory passed as argument t.
 * the array must be big enough for p->vertexcnt pointers to struct vertex.
 */
static void polygon_ysorted_vertex(struct polygon* p, struct vertex** t)
{
	struct vertex* cursor;
	size_t i;

	/* Fill the array */
	cursor = p->v_list;
	for (i = 0; i < p->vertexcnt; i++) {
		t[i] = cursor;
		cursor = cursor->next;
	}

	/* Sort the array */
	qsort(t, p->vertexcnt, sizeof(*t), polygon_ysorted_compare);
}

struct active_edge {
	struct vertex* vymin;
	struct vertex* vymax;
	int x_inter;
	int x_err;
	int divfix_start;
	int divfix_end;
	int dx;
	int dy;
	struct active_edge* next;
};

struct active_edge* active_edge_merge_lists(struct active_edge* ael1, struct active_edge* ael2)
{
	struct active_edge *ret, *retcur;

	if (ael1->x_inter < ael2->x_inter)
	{
		ret = ael1;
		ael1 = ael1->next;
	}
	else if (ael1->x_inter > ael2->x_inter)
	{
		ret = ael2;
		ael2 = ael2->next;
	}
	/* In case of equality, sort by vymax->x */
	else if (ael1->vymax->x <= ael2->vymax->x)
	{
		ret = ael1;
		ael1 = ael1->next;
	}
	else
	{
		ret = ael2;
		ael2 = ael2->next;
	}

	retcur = ret;

	while (ael1 != NULL && ael2 != NULL)
	{
		if (ael1->x_inter < ael2->x_inter)
		{
			retcur->next = ael1;
			ael1 = ael1->next;
		}
		else if (ael1->x_inter > ael2->x_inter)
		{
			retcur->next = ael2;
			ael2 = ael2->next;
		}
		/* In case of equality, sort by vymax->x */
		else if (ael1->vymax->x <= ael2->vymax->x)
		{
			retcur->next = ael1;
			ael1 = ael1->next;
		}
		else
		{
			retcur->next = ael2;
			ael2 = ael2->next;
		}

		retcur = retcur->next;
	}

	while (ael1 != NULL)
	{
		retcur->next = ael1;
		ael1 = ael1->next;
		retcur = retcur->next;
	}

	while (ael2 != NULL)
	{
		retcur->next = ael2;
		ael2 = ael2->next;
		retcur = retcur->next;
	}

	retcur->next = NULL;

	return ret;
}

struct active_edge* active_edge_sort(struct active_edge* ael, size_t size)
{
	struct active_edge *ael1, *ael2;
	size_t size1, size2;

	if (size == 1)
		ael->next = NULL;

	if (size <= 1)
		return ael;

	/* TODO: Améliorer cet algo en découpant en "runs" */
	/* Coupe la liste en deux */
	ael1 = ael;
	size1 = size / 2;

	ael2 = ael1;
	size2 = size;
	while (size2 != size - size1)
	{
		ael2 = ael2->next;
		size2--;
	}

	/* Trie les deux moitiés de liste */
	ael1 = active_edge_sort(ael1, size1);
	ael2 = active_edge_sort(ael2, size2);

	/* Merge les deux moitiés de listes */
	return active_edge_merge_lists(ael1, ael2);
}

void active_edge_init(struct vertex* vymin, struct vertex* vymax, struct active_edge* next, struct active_edge* ae)
{
	memset(ae, 0, sizeof(*ae));

	ae->vymin = vymin;
	ae->vymax = vymax;
	ae->x_inter = ae->vymin->x;
	ae->dx = ae->vymax->x - ae->vymin->x;
	ae->dy = ae->vymax->y - ae->vymin->y;

	if (ae->dx > 0)
	{
		if (ae->dx >= ae->dy)
		{
			ae->x_err = -ae->dx;
			ae->divfix_start = 2 * ae->dx;
			ae->divfix_end = 2 * ae->dy;
		}
		else
		{
			ae->x_err = ae->dy; // OK avec la version normale
			ae->divfix_start = 0;
			ae->divfix_end = 0;
		}
	}
	else
	{
		if (-ae->dx >= ae->dy)
		{
			ae->x_err = -ae->dx;
			ae->divfix_start = -2 * ae->dy + 1;
			ae->divfix_end = 2 * ae->dx;
		}
		else
		{
			ae->x_err = -ae->dy; // OK avec la version normale
			ae->divfix_start = 0;
			ae->divfix_end = 0;
		}
	}
	ae->dx *= 2;
	ae->dy *= 2;
	ae->next = next;
}

static void polygon_fill(struct polygon* p, Image* img)
{
	struct vertex** yvertex = NULL;
	size_t yvertex_bound_idx;
	int ymin, ymax, y;
	struct active_edge* ael = NULL;
	size_t ael_size = 0;

	yvertex = malloc(p->vertexcnt * sizeof(*yvertex));
	if (yvertex == NULL)
	{
		perror("malloc");
		exit(EXIT_FAILURE);
	}

	polygon_ysorted_vertex(p, yvertex);

	/* TODO: mettre des segments à la place des vertex? */

	ymin = yvertex[0]->y;
	ymax = yvertex[p->vertexcnt - 1]->y;

	/* Construit la liste initiale des active_edge */
	/* TODO: merger ça avec l'ajout des sommets en ^ */
	for (yvertex_bound_idx = 0; yvertex_bound_idx < p->vertexcnt && yvertex[yvertex_bound_idx]->y == ymin; yvertex_bound_idx++)
	{
		/* Les sommets du haut sont forcément des angles,
		 * donc vont par deux */
		struct active_edge *ae1, *ae2;

		ae1 = malloc(sizeof(*ae1));
		ae2 = malloc(sizeof(*ae2));
		if (ae1 == NULL || ae2 == NULL)
		{
			perror("malloc");
			exit(EXIT_FAILURE);
		}

		active_edge_init(yvertex[yvertex_bound_idx], yvertex[yvertex_bound_idx]->next, ae2, ae1);
		active_edge_init(yvertex[yvertex_bound_idx], yvertex[yvertex_bound_idx]->prev, ael, ae2);

		ael = ae1;
		ael_size += 2;
	}

	ael = active_edge_sort(ael, ael_size);

	for (y = ymin; y < ymax; y++)
	{
		struct active_edge *aep, *ae, *aen; /* prev, current, next */
		int x;

		aep = NULL;
		ae = ael;

		while (ae != NULL)
		{
			aen = ae->next;

			/*
			 * Remplacement de l'edge qu'on quitte par celui qui le remplace
			 */
			if (ae->vymax->y == y)
			{
				if (ae->vymax->next->y > y)
					active_edge_init(ae->vymax, ae->vymax->next, ae->next, ae);
				else if (ae->vymax->prev->y > y)
					active_edge_init(ae->vymax, ae->vymax->prev, ae->next, ae);
				else
				{
					/* On a atteint un sommet en V dans lequel on coloriait,
					 * on élimine le couple d'arête ae+aen */
					if (aep == NULL)
						ael = aen->next;
					else
						aep->next = aen->next;

					free(ae);
					ae = aen->next;
					free(aen);
					ael_size -= 2;
					continue;
				}
			}

			if (aen->vymax->y == y)
			{
				if (aen->vymax->next->y > y)
					active_edge_init(aen->vymax, aen->vymax->next, aen->next, aen);
				else if (aen->vymax->prev->y > y)
					active_edge_init(aen->vymax, aen->vymax->prev, aen->next, aen);
				else {
					/* On a atteint un sommet en V dans lequel on ne coloriait pas,
					 * on élimine le couple d'arête aen+aen->next */

					ae->next = aen->next->next;

					free(aen->next);
					free(aen);
					ael_size -= 2;
					continue;
				}
			}

			for (x = ae->x_inter + 1; x < aen->x_inter; x++)
				I_plot(img, x, y);

			ae->x_err += ae->dx;
			{
				int xdiff = (ae->x_err + ae->divfix_start) / ae->dy;
				ae->x_inter += xdiff;
				ae->x_err -= xdiff * ae->dy;
			}

			aen->x_err += aen->dx;
			{
				int xdiff = (aen->x_err + aen->divfix_end) / aen->dy;
				aen->x_inter += xdiff;
				aen->x_err -= xdiff * aen->dy;
			}

			aep = aen;
			ae = aen->next;
		}

		/* Ajout des sommets en ^ */
		while (yvertex_bound_idx < p->vertexcnt && yvertex[yvertex_bound_idx]->y == y)
		{
			struct vertex *v = yvertex[yvertex_bound_idx];
			if (v->next->y > y && v->prev->y > y)
			{
				struct active_edge *ae1, *ae2;

				ae1 = malloc(sizeof(*ae1));
				ae2 = malloc(sizeof(*ae2));
				if (ae1 == NULL || ae2 == NULL)
				{
					perror("malloc");
					exit(EXIT_FAILURE);
				}

				active_edge_init(yvertex[yvertex_bound_idx], yvertex[yvertex_bound_idx]->next, ae2, ae1);
				active_edge_init(yvertex[yvertex_bound_idx], yvertex[yvertex_bound_idx]->prev, ael, ae2);

				ael = ae1;
				ael_size += 2;
			}
			yvertex_bound_idx++;
		}

		/* re-sort the active edge list */
		/* TODO: only sort when new vertex have been added */
		ael = active_edge_sort(ael, ael_size);

	}

	while (ael != NULL) {
		struct active_edge* ae = ael->next;
		free(ael);
		ael = ae;
	}
	free(yvertex);
}

void drawing_rasterize(struct drawing *d, Image *img, enum mode current_mode)
{
	struct polygon* p;
	Color black = C_new(0,0,0);
	Color white = C_new(1,1,1);
	Color green = C_new(0,1,0);
	Color red = C_new(1,0,0);

	I_fill(img, black);
	I_changeColor(img, white);

	p = d->p_list;
	while(p != NULL)
	{
		polygon_rasterize(p, img);
		if (p->is_filled)
			polygon_fill(p, img);
		if(current_mode == VERTEX && d->p_active != NULL && d->v_selected != NULL)
		{
			I_changeColor(img, green);
			segment_rasterize(img, d->v_selected->x-5, d->v_selected->y-5, d->v_selected->x+5, d->v_selected->y-5);
			segment_rasterize(img, d->v_selected->x-5, d->v_selected->y-5, d->v_selected->x-5, d->v_selected->y+5);
			segment_rasterize(img, d->v_selected->x+5, d->v_selected->y+5, d->v_selected->x+5, d->v_selected->y-5);
			segment_rasterize(img, d->v_selected->x+5, d->v_selected->y+5, d->v_selected->x-5, d->v_selected->y+5);
		}

		if(current_mode == EDGE && d->p_active != NULL && d->v_selected != NULL && d->v_selected->next != NULL)
		{
			I_changeColor(img, red);
			segment_rasterize(img, d->v_selected->x, d->v_selected->y, d->v_selected->next->x, d->v_selected->next->y);
		}
		p = p->next;
	}
}

/* Renvoie le point le plus proche du clic */
struct vertex* closestVertex(struct polygon *p, int x, int y)
{
	if (p != NULL && p->v_list != NULL)
	{
		//Initialisation des paramètres de recherche
		struct vertex* ret = p->v_list;
		int min = (x - ret->x) * (x - ret->x) + (y - ret->y) * (y - ret->y);
		int tmp;
		struct vertex* cursor = ret->next;

		do
		{
			tmp = (x - cursor->x) * (x - cursor->x) + (y - cursor->y) * (y - cursor->y);
			if (tmp < min)
			{
				min = tmp;
				ret = cursor;
			}
			cursor = cursor->next;
		} while (cursor != p->v_last->next); // Tant qu'on n'a pas étudié chaque sommet de p

		return ret;
	}
	else
	{
		perror("404 not found");
		exit(EXIT_FAILURE);
	}
}

/* Renvoie l'arête la plus proche du clic */
struct vertex* closestEdge(struct polygon *p, int x, int y)
{
	struct vertex* ret = closestVertex(p, x, y);

	int d_next = 0;
	int d_prev = 0;

	if (ret->prev != NULL)
		d_prev = (x - ret->prev->x) * (x - ret->prev->x) + (y - ret->prev->y) * (y - ret->prev->y);
	if (ret->next != NULL)
		d_next = (x - ret->next->x) * (x - ret->next->x) + (y - ret->next->y) * (y - ret->next->y);

	if (d_next == 0 || d_prev <= d_next)
		return ret->prev;
	else if (d_prev == 0 || d_prev > d_next)
		return ret;
	else
	{
		perror("404 not found");
		exit(EXIT_FAILURE);
	}
}

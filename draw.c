#include "draw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * Affiche un message d'erreur correspondant à errno précédé
 */
static void system_error(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

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
 * Crée un nouveau point en milieu de liste
 */
struct vertex* polygon_insert_vertex_after(struct polygon* poly,
                                           struct vertex* v, int x, int y)
{
	// Allocation
	struct vertex* new = malloc(sizeof(*new));

	memset(new, 0, sizeof(*new));

	new->x = x;
	new->y = y;

	new->prev = v;
	new->next = v->next;
	v->next = new;

	if (new->next != NULL)
		new->next->prev = new;

	if (poly->v_last == v)
		poly->v_last = new;

	poly->vertexcnt++;

	return new;
}


/*
 * Supprime un sommet de la liste.
 */
struct vertex* polygon_remove_vertex(struct polygon* poly,
                                     struct vertex* victim)
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

	/* On ne ferme pas un polygone de moins de 3 sommets */
	if (polygon_is_closed(poly) && poly->vertexcnt < 3)
		polygon_toggle_close(poly);

	return poly->v_list;
}

/*
 * Ferme un polygone overt et inversement.
 * Renvoie 1 si le polygone vient d'être fermé, 0 sinon.
 */
int polygon_toggle_close(struct polygon* poly)
{
	if (poly->v_list == NULL)
		return 0;

	// Polygone ouvert -> on le ferme
	if (poly->v_list->prev == NULL && poly->vertexcnt >= 3)
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
		system_error("malloc");

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

	/* TODO: simplifier le code */
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


/* Internal function for active_edge_merge_lists.
 * Do NOT call with any argument NULL */
static void active_edge_pop_smallest(struct active_edge** ael1,
                                     struct active_edge** ael2,
                                     struct active_edge** ret)
{
	if (*ael1 == NULL && *ael2 == NULL) {
		*ret = NULL;
	}
	else if (*ael2 == NULL)
	{
		*ret = *ael1;
		*ael1 = (*ael1)->next;
	}
	else if (*ael1 == NULL)
	{
		*ret = *ael2;
		*ael2 = (*ael2)->next;
	}
	else if ((*ael1)->x_inter < (*ael2)->x_inter)
	{
		*ret = *ael1;
		*ael1 = (*ael1)->next;
	}
	else if ((*ael1)->x_inter > (*ael2)->x_inter)
	{
		*ret = *ael2;
		*ael2 = (*ael2)->next;
	}
	/* In case of equality, sort by vymax->x */
	else if ((*ael1)->vymax->x <= (*ael2)->vymax->x)
	{
		*ret = *ael1;
		*ael1 = (*ael1)->next;
	}
	else
	{
		*ret = *ael2;
		*ael2 = (*ael2)->next;
	}
}

static struct active_edge* active_edge_merge_lists(struct active_edge* ael1,
                                                   struct active_edge* ael2)
{
	struct active_edge *ret, *retcur;

	active_edge_pop_smallest(&ael1, &ael2, &ret);
	retcur = ret;

	while (ael1 != NULL || ael2 != NULL)
	{
		active_edge_pop_smallest(&ael1, &ael2, &retcur->next);
		retcur = retcur->next;
	}

	retcur->next = NULL;

	return ret;
}

static struct active_edge* active_edge_sort(struct active_edge* ael, size_t size)
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

static void active_edge_init(struct vertex* vymin, struct vertex* vymax,
                             struct active_edge* next, struct active_edge* ae)
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
			ae->x_err = ae->dy;
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
			ae->x_err = -ae->dy;
			ae->divfix_start = 0;
			ae->divfix_end = 0;
		}
	}
	ae->dx *= 2;
	ae->dy *= 2;
	ae->next = next;
}

/* Cette structure représente l'état du scanline à mettre à jour et à
 * transporter entre chaque ligne */
struct scanline_state {
	struct polygon* poly;

	/* Liste des sommets du polygone */
	struct vertex** yvertex;
	/* Indice de yvertex qui représente la borne max de la zone courante */
	size_t yv_idx;

	/* Liste des arêtes actives */
	struct active_edge* ael;
	size_t ael_size;
};


/* Ajoute à la liste des active edge les arêtes qui participent à un sommet en ^
 * et dont le ymin est égale à y */
static void scan_line_add_vertex(struct scanline_state* sls, int y)
{
	/* Raccourcis */
	struct vertex** yv = sls->yvertex;
	struct active_edge* ael = sls->ael;
	size_t ael_size = sls->ael_size;

	while (sls->yv_idx < sls->poly->vertexcnt && yv[sls->yv_idx]->y == y)
	{
		struct vertex *v = yv[sls->yv_idx];
		struct active_edge *aetmp;

		/* N'ajoute que les sommets en ^
		 * Même ceux dont un des côtés est horizontal.
		 * Les sommets "traversants" la droite "y" sont déjà
		 * gérés. */

		if (v->next->y > y && v->prev->y >= y)
		{
			aetmp = malloc(sizeof(*aetmp));
			if (aetmp == NULL)
				system_error("malloc");

			active_edge_init(v, v->next, ael, aetmp);
			ael = aetmp;
			ael_size++;
		}
		if (v->prev->y > y && v->next->y >= y)
		{
			aetmp = malloc(sizeof(*aetmp));
			if (aetmp == NULL)
				system_error("malloc");

			active_edge_init(v, v->prev, ael, aetmp);
			ael = aetmp;
			ael_size++;
		}

		sls->yv_idx++;
	}

	/* Trie les active edge uniquement si on en a rajouté */
	if (ael_size != sls->ael_size) {
		sls->ael = active_edge_sort(ael, ael_size);
		sls->ael_size = ael_size;
	}
}

/*  Remplacement de l'arête active ae par la suivante dans l'ordre des y
 *  croissants */
static void scan_line_active_edge_next(struct scanline_state* sls,
                                       struct active_edge** ae,
                                       struct active_edge** aep)
{
	/* Raccourcis */
	struct active_edge* a = *ae;
	struct active_edge* next = a->next;
	struct vertex* vymax = a->vymax;
	int y = vymax->y;

	if (vymax->next->y > y)
		active_edge_init(vymax, vymax->next, next, a);
	else if (vymax->prev->y > y)
		active_edge_init(vymax, vymax->prev, next, a);
	else
	{
		/*
		 * On a atteint un sommet en V (dont une des deux branches peut
		 * être horizontale) on élimine l'arête. Le nombre d'arête
		 * devient impaire mais c'est pas grave, l'autre arête sera
		 * supprimée lors d'une prochaine itération.
		 */
		*aep = next;
		*ae = next;
		free(a);
		sls->ael_size--;
	}
}

/* Balaye une ligne du polygone */
static void scan_line(struct scanline_state* sls, int y, Image* img)
{
	struct active_edge *aep, *ae, *aen; /* prev, current, next */
	int x;

	/* Raccourcis */
	struct active_edge* ael;
	size_t ael_size;


	/* Ajoute les arêtes qui participent à un sommet en ^ */
	scan_line_add_vertex(sls, y);
	ael = sls->ael;
	ael_size = sls->ael_size;

	/* TODO: Ne trier que quand des arêtes ont été ajoutées
	 * Et échanger les arêtes qui ont été croisées sans appeller active_edge_sort */
	ael = active_edge_sort(ael, ael_size);


	aep = NULL;
	ae = ael;

	/* Parcourt toutes les paires d'arêtes actives */
	while (ae != NULL)
	{
		/* Supprime les premières arêtes actives si on a atteint le
		 * bas */
		while (ae != NULL && ae->vymax->y == y)
		{
			if (aep == NULL)
				scan_line_active_edge_next(sls, &ae, &ael);
			else
				scan_line_active_edge_next(sls, &ae, &aep->next);

			ael_size = sls->ael_size;
		}

		if (ae == NULL)
			break;

		aen = ae->next;

		/* Supprime les arêtes actives de fin de segment si on a atteint
		 * le bas */
		while (aen->vymax->y == y)
		{
			scan_line_active_edge_next(sls, &aen, &ae->next);
			ael_size = sls->ael_size;
			aen = ae->next;
		}

		for (x = ae->x_inter + 1; x < aen->x_inter; x++)
			I_plot(img, x, y);

		/* TODO: swaper ae avec ses voisins si on vient de les
		 * croiser.
		 * /!\ Attention aux modifications de la liste ael
		 * pendant son parcourt. */
		ae->x_err += ae->dx;
		{
			int xdiff = (ae->x_err + ae->divfix_start) / ae->dy;
			ae->x_inter += xdiff;
			ae->x_err -= xdiff * ae->dy;
		}

		/* TODO: idem aen */
		aen->x_err += aen->dx;
		{
			int xdiff = (aen->x_err + aen->divfix_end) / aen->dy;
			aen->x_inter += xdiff;
			aen->x_err -= xdiff * aen->dy;
		}

		aep = aen;
		ae = aen->next;
	}

	sls->ael = ael;
	sls->ael_size = ael_size;
}

static void polygon_fill(struct polygon* p, Image* img)
{
	int ymin, ymax, y;
	struct vertex** yvertex;
	struct scanline_state sls;

	memset(&sls, 0, sizeof(sls));

	yvertex = malloc(p->vertexcnt * sizeof(*yvertex));
	if (yvertex == NULL)
		system_error("malloc");

	polygon_ysorted_vertex(p, yvertex);

	ymin = yvertex[0]->y;
	ymax = yvertex[p->vertexcnt - 1]->y;

	sls.poly = p;
	sls.yvertex = yvertex;
	sls.yv_idx = 0;
	sls.ael = NULL;
	sls.ael_size = 0;

	for (y = ymin; y <= ymax; y++)
		scan_line(&sls, y, img);

	free(yvertex);
}

static void vertex_square_around(struct vertex* v, int l, Image* img)
{
	l = l / 2;
	segment_rasterize(img, v->x - l, v->y - l, v->x + l, v->y - l);
	segment_rasterize(img, v->x - l, v->y - l, v->x - l, v->y + l);
	segment_rasterize(img, v->x + l, v->y + l, v->x + l, v->y - l);
	segment_rasterize(img, v->x + l, v->y + l, v->x - l, v->y + l);
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
	while (p != NULL)
	{
		polygon_rasterize(p, img);
		if (p->is_filled)
			polygon_fill(p, img);

		p = p->next;
	}

	if (current_mode == VERTEX && d->p_active != NULL && d->v_selected != NULL)
	{
		I_changeColor(img, green);
		vertex_square_around(d->v_selected, 10, img);
		I_changeColor(img, white);
	}

	if (current_mode == EDGE && d->p_active != NULL &&
	   d->v_selected != NULL && d->v_selected->next != NULL)
	{
		I_changeColor(img, red);
		segment_rasterize(img, d->v_selected->x, d->v_selected->y,
		                  d->v_selected->next->x, d->v_selected->next->y);
		I_changeColor(img, white);
	}
}

/*
 * Supprime un polygone d'une drawing et le désalloue
 */
void drawing_remove_polygon(struct drawing *d, struct polygon *p)
{
	struct polygon* q;

	while (p->vertexcnt > 0)
		polygon_remove_vertex(p, p->v_list);

	if (d->p_list == p)
		d->p_list = p->next;

	for (q = d->p_list; q != NULL; q = q->next)
		if (q->next == p)
			q->next = p->next;

	if (q == NULL)
		fprintf(stderr, "Suppression d'un polygon étranger au dessin\n");

	free(p);
}

/*
 * Libère et vide toutes les structures d'une struct drawing
 */
void drawing_free(struct drawing *d)
{
	while (d->p_list != NULL)
		drawing_remove_polygon(d, d->p_list);

	memset(d, 0, sizeof(*d));
}

/*
 * Retourne le carré de la distance entre deux points
 */
static long dist2(int x1, int y1, int x2, int y2)
{
	int dx = x2 - x1;
	int dy = y2 - y1;
	return dx * dx + dy * dy;
}

/* Renvoie le point le plus proche du clique */
struct vertex* closestVertex(struct polygon *p, int x, int y)
{
	struct vertex* ret = NULL;
	long min = -1, tmp;
	struct vertex* cursor = NULL;
	size_t i;

	if (p == NULL || p->v_list == NULL)
	{
		fprintf(stderr, "404 not found\n");
		return NULL;
	}

	cursor = p->v_list;
	for (i = 0; i < p->vertexcnt; i++)
	{
		tmp = dist2(cursor->x, cursor->y, x, y);
		if (min == -1 || tmp < min)
		{
			min = tmp;
			ret = cursor;
		}
		cursor = cursor->next;
	}

	return ret;
}

static long edge_point_dist2(struct vertex* e1, int x, int y)
{
	struct vertex* e2 = e1->next;
	long l2 = dist2(e2->x, e2->y, e1->x, e1->y);

	/* Arête de taille nulle */
	if (l2 == 0)
		return dist2(e1->x, e1->y, x, y);

	long tl2 = ((x - e1->x) * (e2->x - e1->x) + (y - e1->y) * (e2->y - e1->y));

	/* Point qui dépasse du côté de e1 */
	if (tl2 < 0)
		return dist2(e1->x, e1->y, x, y);

	/* Point qui dépasse du côté de e2 */
	if (tl2 > l2)
		return dist2(e2->x, e2->y, x, y);

	/* Distance du projeté orthogonal sur la droite */
	long hx = e1->x + tl2 * (e2->x - e1->x) / l2;
	long hy = e1->y + tl2 * (e2->y - e1->y) / l2;
	return dist2(hx, hy, x, y);
}

/* Renvoie l'arête la plus proche du clique */
struct vertex* closestEdge(struct polygon *p, int x, int y)
{
	long mindist = -1;
	struct vertex* ret = NULL;
	long tmpdist;
	struct vertex* v = NULL;
	size_t i;

	if (p->v_list == NULL)
		return NULL;

	v = p->v_list;
	for (i = 0; i < p->vertexcnt; i++)
	{
		tmpdist = edge_point_dist2(v, x, y);
		if (mindist == -1 || tmpdist < mindist)
		{
			mindist = tmpdist;
			ret = v;
		}

		v = v->next;
	}

	return ret;
}

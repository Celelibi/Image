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

	/* Champs utilisés pour le natural merge sort */
	size_t runsize;
	struct active_edge* nextrun;
};


static int active_edge_cmp(struct active_edge* ae1, struct active_edge* ae2)
{
	int ret;

	ret = ae1->x_inter - ae2->x_inter;
	if (ret != 0)
		return ret;

	ret = ae1->vymax->x - ae2->vymax->x;
	return ret;
}

/* Fonction interne pour active_edge_merge_runs
 * Ne PAS appeller avec un argument NULL */
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
	else if (active_edge_cmp(*ael1, *ael2) <= 0)
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

/* Merge deux runs, et PAS deux listes de run. Et produit un seul run. */
static struct active_edge* active_edge_merge_runs(struct active_edge* run1,
                                                  struct active_edge* run2)
{
	struct active_edge *ret, *retcur;
	size_t totalsize = run1->runsize + run2->runsize;

	/* Supprime les infos de "run", on reconstruira un run à la fin.
	 * /!\ Les états intermédiaires ne forment pas de run valide. */
	/* Note: On ne merge que des run uniques, donc nextrun == NULL */
	/*run1->nextrun = NULL;
	run2->nextrun = NULL;*/
	run1->runsize = 0;
	run2->runsize = 0;

	active_edge_pop_smallest(&run1, &run2, &ret);
	retcur = ret;

	while (run1 != NULL || run2 != NULL)
	{
		active_edge_pop_smallest(&run1, &run2, &retcur->next);
		retcur = retcur->next;
	}

	retcur->next = NULL;

	/* Forme un run valide */
	ret->nextrun = NULL;
	ret->runsize = totalsize;

	return ret;
}

/* Trie une liste de "run" au lieu d'une liste d'edge
 * "size" est le nombre d'active_edge, pas le nombre de runs */
static struct active_edge* active_edge_sort_runs(struct active_edge* runl,
                                                 size_t size)
{
	struct active_edge *prevrun, *runl1, *runl2;
	size_t size1, size2;

	/* Un run unique est déjà trié. */
	if (runl == NULL || runl->nextrun == NULL)
		return runl;

	/* Coupe la liste de runs en deux liste contenant à peu près le même
	 * nombre d'active edge. */
	runl1 = runl;

	size1 = 0;
	size2 = size;
	prevrun = runl1;
	runl2 = runl1;
	while (runl2 != NULL && size1 < size2 - runl2->runsize)
	{
		size1 += runl2->runsize;
		size2 -= runl2->runsize;
		prevrun = runl2;
		runl2 = runl2->nextrun;
	}

	prevrun->nextrun = NULL;

	/* Trie les deux listes de run et produit deux runs triés (et pas deux
	 * listes de run) */
	runl1 = active_edge_sort_runs(runl1, size1);
	runl2 = active_edge_sort_runs(runl2, size2);

	/* Merge les deux moitiés de listes */
	return active_edge_merge_runs(runl1, runl2);
}

/* Remplit le champ runsize de ael et retourne le premier élément ne faisant pas
 * parti du run */
static struct active_edge* active_edge_build_cur_run(struct active_edge* ael)
{
	size_t size;
	struct active_edge *ae, *aep;

	if (ael == NULL)
		return NULL;

	size = 1;
	aep = ael;
	ae = aep->next;
	while (ae != NULL && active_edge_cmp(aep, ae) < 0)
	{
		size++;
		aep = ae;
		ae = ae->next;
	}

	aep->next = NULL;
	ael->runsize = size;
	return ae;
}

static struct active_edge* active_edge_build_runs(struct active_edge* ael)
{
	struct active_edge *run;
	struct active_edge *ae;

	run = ael;
	while ((ae = active_edge_build_cur_run(run)))
	{
		run->nextrun = ae;
		run = ae;
	}

	run->nextrun = NULL;

	return ael;
}

static struct active_edge* active_edge_sort(struct active_edge* ael, size_t size)
{
	struct active_edge* runs;
	runs = active_edge_build_runs(ael);
	runs = active_edge_sort_runs(runs, size);
	runs->runsize = 0;
	runs->nextrun = NULL;
	return runs;
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
	int ael_need_sort;
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
		sls->ael = ael;
		sls->ael_need_sort = 1;
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

/* Échange ae et son successeur dans la liste des active edge s'ils ne sont pas
 * triés selon leur x_inter. Le successeur doit exister.
 * paep est un pointeur sur le pointeur vers ae. Généralement le champ "next" du
 * active_edge précédent. Parfois un pointeur sur la tête de liste ael.
 * Retourne le plus petit entre ae et ae->next */
static struct active_edge* scan_line_ae_swap(struct scanline_state* sls,
                                             struct active_edge** paep,
                                             struct active_edge* ae)
{
	struct active_edge* aen = ae->next;
	struct active_edge* aenn = aen->next;

	if (ae->x_inter > aen->x_inter)
	{
		/* Re-link the active edge list */
		ae->next = aenn;
		aen->next = ae;
		*paep = aen;

		/* Vérifie si on a besoin d'un vrai tri. */
		if (*paep != NULL && (*paep)->x_inter > aen->x_inter)
			sls->ael_need_sort = 1;

		return aen;
	}

	return ae;
}

/* Balaye une ligne du polygone */
static void scan_line(struct scanline_state* sls, int y, Image* img)
{
	/* prevprev, prev, current, next */
	struct active_edge *aepp, *aep, *ae, *aen;
	int x;

	/* Raccourcis */
	struct active_edge* ael;


	/* Ajoute les arêtes qui participent à un sommet en ^ */
	scan_line_add_vertex(sls, y);
	ael = sls->ael;

	if (sls->ael_need_sort)
	{
		ael = active_edge_sort(ael, sls->ael_size);
		sls->ael_need_sort = 0;
	}


	aepp = NULL;
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
		}

		if (ae == NULL)
			break;

		aen = ae->next;

		/* Supprime les arêtes actives de fin de segment si on a atteint
		 * le bas */
		while (aen->vymax->y == y)
		{
			scan_line_active_edge_next(sls, &aen, &ae->next);
			aen = ae->next;
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

		/* Tente de maintenir l'ordre
		 * La plupart du temps, seules 2 arrêtes se croisent, donc ces
		 * inversions sont suffisantes. Si elles ne le sont pas, on
		 * lancera un vrai tri.
		 * /!\ Attention, on ne touche pas à la suite de la liste après
		 * aen */

		/* 1) Échange ae et son prédécesseur si nécessaire */
		if (aep != NULL)
		{
			if (aepp != NULL)
				aep = scan_line_ae_swap(sls, &aepp->next, aep);
			else
				aep = scan_line_ae_swap(sls, &ael, aep);

			ae = aep->next;
		}

		/* 2) Échange aen et ae si nécessaire */
		if (aep != NULL)
			ae = scan_line_ae_swap(sls, &aep->next, ae);
		else
			ae = scan_line_ae_swap(sls, &ael, ae);

		aen = ae->next;

		aepp = ae;
		aep = aen;
		ae = aen->next;
	}

	sls->ael = ael;
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
	sls.ael_need_sort = 0;

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

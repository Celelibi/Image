#ifndef DRAW_H
#define DRAW_H

#include "Image.h"

/*
* Structure de liste chaînée pour stocker des coordonnées
*/

struct vertex {
	int x;
	int y;
	struct vertex* next;
	struct vertex* prev;
};

/*
* Structure de liste chaînée pour stocker une liste de points
*/

struct polygon {
	struct vertex* v_list;
	int is_filled;
	struct polygon* next;
};

/*
* Liste de polygones
*/

struct drawing {
	struct polygon* p_list;
	struct polygon* p_active;
};

/*
* Des fonctions pour les gouverner tous, et dans la RAM les allouer...
*/

struct vertex* polygon_append_vertex(struct vertex* list, int x, int y);
struct vertex* polygon_remove_vertex(struct vertex* list, struct vertex* victim);

/* Ferme un polygone overt et inversement. Renvoi 1 si le polygone vient d'être fermé, 0 sinon. */
int polygon_toggle_close(struct polygon* poly);
int polygon_is_closed(struct polygon* poly);

/* Ajoute un polygone vide à la fin de la liste et le renvoi et le défini comme polygone actif */
void drawing_new_polygon(struct drawing *d);

void segment_rasterize(Image *img, int xA, int yA, int xB, int yB);
void polygon_rasterize(struct polygon *p, Image *img);
void drawing_rasterize(struct drawing *d, Image *img);

/*
* À ajouter plus tard :
* Insertion d'un sommet en plein milieu de la liste
*/

#endif

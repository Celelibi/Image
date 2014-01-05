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
};

/*
* Des fonctions pour les gouverner tous, et dans la RAM les allouer...
*/

struct vertex* polygon_append_vertex(struct vertex* list, int x, int y);
struct vertex* polygon_remove_vertex(struct vertex* list, struct vertex* victim);
void polygon_rasterize(struct polygon *p, Image *img);
void drawing_rasterize(struct drawing *d, Image *img); 
void segment_rasterize(Image *img, int xA, int yA, int xB, int yB);

/*
* À ajouter plus tard :
* Insertion d'un sommet en plein milieu de la liste
*/

#endif

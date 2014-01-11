#ifndef DRAW_H
#define DRAW_H

#include "Image.h"

/*
* Différents modes de fonctionnement du programme
*/
enum mode {APPEND, VERTEX, EDGE};

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
	struct vertex* v_last;
	size_t vertexcnt;
	int is_filled;
	struct polygon* next;
};

/*
* Liste de polygones
*/

struct drawing {
	struct polygon* p_list;
	struct polygon* p_active;
	struct vertex* v_selected;
};

/* Crée un nouveau point en fin de liste */
struct vertex* polygon_append_vertex(struct polygon* poly, int x, int y);

/* Crée un nouveau point en milieu de liste */
struct vertex* polygon_insert_vertex_after(struct polygon* poly, struct vertex* v, int x, int y);

/* Supprime un point*/
struct vertex* polygon_remove_vertex(struct polygon* poly, struct vertex* victim);

/* Ferme un polygone ouvert et inversement. Renvoie 1 si le polygone vient d'être fermé, 0 sinon. */
int polygon_toggle_close(struct polygon* poly);

/* Dit si le polygone est fermé ou non */
int polygon_is_closed(struct polygon* poly);

/* Ajoute un polygone vide à la fin de la liste et le renvoie et le définit comme polygone actif */
void drawing_new_polygon(struct drawing *d);

/* Trace une droite de Bresenham entre deux points donnés */
void segment_rasterize(Image *img, int xA, int yA, int xB, int yB);

/* Dessine tous les points d'un polygone */
void polygon_rasterize(struct polygon *p, Image *img);

/* Dessine tous les polygones, change la couleur du point ou segment sélectionné */
void drawing_rasterize(struct drawing *d, Image *img, enum mode current_mode);

/* Supprime un polygone d'une drawing et le désalloue */
void drawing_remove_polygon(struct drawing *d, struct polygon *p);

/* Libère et vide toutes les structures d'une struct drawing */
void drawing_free(struct drawing *d);

/* Renvoie le point le plus proche du clique */
struct vertex* closestVertex(struct polygon *p, int x, int y);

/* Renvoie l'arête la plus proche du clique */
struct vertex* closestEdge(struct polygon *p, int x, int y);

/*
* À ajouter si nécessaire :
* Insertion d'un sommet en plein milieu de la liste
*/

#endif

#ifndef DRAW_H
#define DRAW_H

/*
* Structure de liste chaînée pour stocker des coordonnées !
*/

struct point {
	int x;
	int y;
	struct point* next;
};
typedef struct point sommet;

/*
* Des fonctions pour les gouverner tous, et dans la RAM les allouer...
*/

sommet* sommet_add(sommet* list, int x, int y);
sommet* sommet_get_last(sommet* list);
sommet* sommet_remove(sommet* list, sommet* victim);

/*
* À ajouter plus tard :
* Insertion d'un sommet en plein milieu de la liste
*/

#endif

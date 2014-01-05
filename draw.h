#ifndef DRAW_H
#define DRAW_H

/*
* Structure de liste chaînée pour stocker des coordonnées !
*/

struct sommet {
	int x;
	int y;
	struct sommet* next;
};

/*
* Structure représentant un polygone
*/

struct polygon {
	struct sommet* s_list;
	int is_closed;
};

/*
* Structure de liste chaînée pour stocker une liste de polygones !
*/

struct drawing {
	struct polygon* p_list;
};

/*
* Des fonctions pour les gouverner tous, et dans la RAM les allouer...
*/

struct sommet* sommet_add(struct sommet* list, int x, int y);
struct sommet* sommet_get_last(struct sommet* list);
struct sommet* sommet_remove(struct sommet* list, struct sommet* victim);

/*
* À ajouter plus tard :
* Insertion d'un sommet en plein milieu de la liste
*/

#endif

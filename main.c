
/*===============================================================*\

    Vendredi 25 septembre 2013

	Arash Habibi

	main.c

	Un programme equivalent à 02_glut.c et qui ne prend en compte
	que trois événements pour quitter le programme.

\*===============================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <GL/glut.h>
#include <GL/gl.h>

#include "Image.h"
#include "draw.h"

Image *img;
sommet *s_list;

int is_closed = 0;

//------------------------------------------------------------------
//	C'est le display callback. A chaque fois qu'il faut
//	redessiner l'image, c'est cette fonction qui est
//	appelee. Tous les dessins doivent etre faits a partir
//	de cette fonction.
//------------------------------------------------------------------

void display_CB()
{
	glClear(GL_COLOR_BUFFER_BIT);

	/*I_bresenham(img, 0, 0, 100, 100);
	  I_bresenham(img, 100, 100, 300, 200);
	  I_bresenham(img, 300, 200, 400, 150);
	  I_bresenham(img, 400, 150, 350, 90);
	  I_bresenham(img, 350, 90, 0, 0);*/

	if(s_list != NULL) // Si on a au moins un sommet de placé...
	{
		sommet* cursor = s_list;
		while(cursor->next != NULL)
		{
			I_bresenham(img, cursor->x, cursor->y, cursor->next->x, cursor->next->y);

			cursor = cursor->next;
		}

		if(is_closed)
		{
			// Relie le dernier élément au premier
			I_bresenham(img, cursor->x, cursor->y, s_list->x, s_list->y);
		}
	}

	I_draw(img);

	glutSwapBuffers();
}

//------------------------------------------------------------------
// Cette fonction permet de réagir en fonction de la position de
// la souris (x,y), en fonction du bouton de la souris qui a été
// pressé ou relaché.
//------------------------------------------------------------------

void mouse_CB(int button, int state, int x, int y)
{
	// Lors d'un clic sur le bouton gauche de la souris...
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		y = img->_height - y; // inversion
		printf("--> clic détecté à %d, %d\n", x, y);

		// Bresenham entre le point cliqué et celui cliqué précédemment

		if(is_closed)
		{
			printf("--> le polygone est fermé donc je ne fais rien\n");
			return;
			/*
			 * Rien pour le moment.
			 * TODO : permettre de stocker plus d'un polygone à la fois,
			 * ou au moins d'effacer le polygone actuel.
			 */
		}

		printf("--> ajout du sommet %d, %d\n", x, y);
		s_list = sommet_add(s_list, x, y);
	}

	// Bouton droit...
	else if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		// relégué au bouton droit
		I_focusPoint(img,x,img->_height-y);
	}

	glutPostRedisplay();
}

//------------------------------------------------------------------
// Cette fonction permet de réagir au fait que l'utilisateur
// presse une touche (non-spéciale) du clavier.
//------------------------------------------------------------------

void keyboard_CB(unsigned char key, int x, int y)
{
	// fprintf(stderr,"key=%d\n",key);
	switch(key)
	{
	case 27: exit(EXIT_FAILURE); break;
	case 'z': I_zoom(img, 2.0); break;
	case 'Z': I_zoom(img, 0.5); break;
	case 'i': I_zoomInit(img); break;
	case 'c':
		is_closed = !is_closed;
		if(is_closed)
			printf("Polygone fermé\n");
		else
			printf("Polygone réouvert\n");
		break;

	default:
		fprintf(stderr,"keyboard_CB : %d : unknown key.\n",key);
	}
	glutPostRedisplay();
}

//------------------------------------------------------------------
// Cette fonction permet de réagir au fait que l'utilisateur
// presse une touche spéciale (F1, F2 ... F12, home, end, insert,
// haut, bas, droite, gauche etc).
//------------------------------------------------------------------

void special_CB(int key, int x, int y)
{
	// int mod = glutGetModifiers();

	int d = 10;

	switch(key)
	{
	case GLUT_KEY_UP    : I_move(img,0,d); break;
	case GLUT_KEY_DOWN  : I_move(img,0,-d); break;
	case GLUT_KEY_LEFT  : I_move(img,d,0); break;
	case GLUT_KEY_RIGHT : I_move(img,-d,0); break;
	default : fprintf(stderr,"special_CB : %d : unknown key.\n",key);
	}
	glutPostRedisplay();
}

//------------------------------------------------------------------------

int main(int argc, char **argv)
{
	int largeur, hauteur;
	int windowPosX = 100, windowPosY = 100;

	if (argc != 3 && argc != 2)
	{
		fprintf(stderr,"\n\nUsage \t: %s <width> <height>\nou",argv[0]);
		fprintf(stderr,"\t: %s <ppmfilename> \n\n",argv[0]);
		exit(EXIT_FAILURE);
	}

	if (argc == 2)
	{
		img = I_read(argv[1]);
		largeur = img->_width;
		hauteur = img->_height;
	}
	else
	{
		largeur = atoi(argv[1]);
		hauteur = atoi(argv[2]);
		img = I_new(largeur,hauteur);

		/*Color rouge = C_new(100,0,0);
		Color blanc = C_new(200,200,255);
		I_fill(img,blanc);
		I_checker(img,rouge,50);*/

		Color black = C_new(0,0,0);
		I_fill(img, black);
	}

	glutInitWindowSize(largeur,hauteur);
	glutInitWindowPosition(windowPosX,windowPosY);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE );
	glutInit(&argc, argv);
	glutCreateWindow(argv[0]);

	glViewport(0, 0, largeur, hauteur);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glOrtho(0,largeur,0,hauteur,-1,1);

	glutDisplayFunc(display_CB);
	glutKeyboardFunc(keyboard_CB);
	glutSpecialFunc(special_CB);
	glutMouseFunc(mouse_CB);
	// glutMotionFunc(mouse_move_CB);
	// glutPassiveMotionFunc(passive_mouse_move_CB);

	glutMainLoop();

	return EXIT_SUCCESS;
}


/*===============================================================*\

    Vendredi 25 septembre 2013

	Arash Habibi

	main.c

	Un programme equivalent à 02_glut.c et qui ne prend en compte
	que trois événements pour quitter le programme.

\*===============================================================*/

#include <stdio.h>
#include <stdlib.h>
#include <GL/freeglut.h>
#include <GL/glut.h>
#include <GL/gl.h>

#include "Image.h"
#include "draw.h"

enum mode current_mode = APPEND;

Image *img;
struct drawing drawing;

//------------------------------------------------------------------
//	C'est le display callback. A chaque fois qu'il faut
//	redessiner l'image, c'est cette fonction qui est
//	appelee. Tous les dessins doivent etre faits a partir
//	de cette fonction.
//------------------------------------------------------------------

void display_CB()
{
	glClear(GL_COLOR_BUFFER_BIT);

	drawing_rasterize(&drawing, img, current_mode);
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
	y = img->_height - y; // inversion

	// Lors d'un clique sur le bouton gauche de la souris...
	if(button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		printf("--> clic détecté à %d, %d\n", x, y);

		if(current_mode == APPEND)
		{
			// Bresenham entre le point cliqué et celui cliqué précédemment

			if (drawing.p_active == NULL)
				drawing_new_polygon(&drawing);

			if(polygon_is_closed(drawing.p_active))
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
			polygon_append_vertex(drawing.p_active, x, y);
		}
		if(current_mode == VERTEX)
			drawing.v_selected = closestVertex(drawing.p_active, x, y);
		if(current_mode == EDGE)
			drawing.v_selected = closestEdge(drawing.p_active, x, y);
	}

	// Bouton droit...
	else if(button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		// relégué au bouton droit
		I_focusPoint(img,x,img->_height-y);
	}

	else if(button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
	{
		if(current_mode == EDGE && drawing.p_active != NULL && drawing.v_selected != NULL && drawing.v_selected->next != NULL)
			polygon_insert_vertex_after(drawing.p_active, drawing.v_selected, x, y);
	}

	glutPostRedisplay();
}

//------------------------------------------------------------------
// Cette fonction permet de réagir au fait que l'utilisateur
// presse une touche (non-spéciale) du clavier.
//------------------------------------------------------------------

void keyboard_CB(unsigned char key, int x, int y)
{
	(void)x;
	(void)y;

	// fprintf(stderr,"key=%d\n",key);
	switch(key)
	{
	case 27: // Echap
		glutLeaveMainLoop();
		break;
	case 127: // Suppr
		if(drawing.p_active != NULL && drawing.p_active->v_list != NULL)
		{
			if(current_mode == APPEND)
				polygon_remove_vertex(drawing.p_active, drawing.p_active->v_last);
			if(current_mode == VERTEX)
			{
				polygon_remove_vertex(drawing.p_active, drawing.v_selected);
				drawing.v_selected = drawing.p_active->v_list;
			}
		}
		break;
	case 'z':
		I_zoom(img, 2.0);
		break;
	case 'Z':
		I_zoom(img, 0.5);
		break;
	case 'i':
		I_zoomInit(img);
		break;
	case 'a':
		current_mode = APPEND;
		break;
	case 'v':
		current_mode = VERTEX;
		if(drawing.p_active != NULL)
			drawing.v_selected = drawing.p_active->v_list;
		break;
	case 'e':
		current_mode = EDGE;
		if(drawing.p_active != NULL)
			drawing.v_selected = drawing.p_active->v_list;
		break;
	case 'c':
		if (polygon_toggle_close(drawing.p_active))
			printf("Polygone fermé\n");
		else
			printf("Polygone réouvert\n");
		break;

	case 'f':
		if (!polygon_is_closed(drawing.p_active))
			printf("Le polygone n'est pas fermé\n");
		else
			drawing.p_active->is_filled ^= 1;
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
	(void)x;
	(void)y;

	int d = 10;

	switch(key)
	{
	case GLUT_KEY_UP:
		if(current_mode == APPEND)
			I_move(img,0,d);
		if(current_mode == VERTEX && drawing.v_selected != NULL)
			(drawing.v_selected->y)++;
		break;
	case GLUT_KEY_DOWN:
		if(current_mode == APPEND)
			I_move(img,0,-d);
		if(current_mode == VERTEX && drawing.v_selected != NULL)
			(drawing.v_selected->y)--;
		break;
	case GLUT_KEY_LEFT:
		if(current_mode == APPEND)
			I_move(img,d,0);
		if(current_mode == VERTEX && drawing.v_selected != NULL)
			(drawing.v_selected->x)--;
		break;
	case GLUT_KEY_RIGHT:
		if(current_mode == APPEND)
			I_move(img,-d,0);
		if(current_mode == VERTEX && drawing.v_selected != NULL)
			(drawing.v_selected->x)++;
		break;
	case GLUT_KEY_PAGE_UP:
		if((current_mode == VERTEX && drawing.v_selected->next != NULL) ||
				(current_mode == EDGE && polygon_is_closed(drawing.p_active)))
			drawing.v_selected = drawing.v_selected->next;
		if(current_mode == EDGE && !polygon_is_closed(drawing.p_active) &&
				drawing.v_selected->next != drawing.p_active->v_last)
			drawing.v_selected = drawing.v_selected->next;
		break;
	case GLUT_KEY_PAGE_DOWN:
		if((current_mode == VERTEX || current_mode == EDGE) && drawing.v_selected->prev != NULL)
			drawing.v_selected = drawing.v_selected->prev;
		break;
	default:
		fprintf(stderr,"special_CB : %d : unknown key.\n",key);
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

	drawing_free(&drawing);

	return EXIT_SUCCESS;
}

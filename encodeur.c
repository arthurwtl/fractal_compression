#include "image_pgm.c"

//code un bloc destination de taille 8 par 8 : posx coordonnée sur la hauteur, posy sur la longueur,
//alpha, beta des variable utilisé dans la regression linéaire, cout le cout et sourcex, sourcey les coordonné du bloc source
struct destination{
	int pos_x;
	int pos_y;
	double alpha;
	double beta;
	int coup;
	int source_x;
	int source_y;
};
typedef struct destination destination;


int main()
{
	image* test1 = create_image(100, 100, 255);
	remplissage(100, test1);
	save_image(test1, "test.pgm");
	free_image(test1);	

	image* test2 = lecture_image("feep.pgm");
	//save_image(test2, "ouiçamarche.pgm");
	free_image(test2);	


	return 1;
}

#ifndef standard_lib
#define standard_lib
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#endif

struct image_s{
    int longueur;
    int hauteur;
	int niveau;
    double** matrice;
};
typedef struct image_s image;

//crée une structure image sans initialiser les pixels
image* create_image(int h, int l, int n){
    image* img = (image*)malloc(sizeof(image));
    assert(img != NULL);

    img->hauteur = h; 
    img->longueur = l;
	img->niveau = n;
	
    img->matrice = (double**)(malloc(sizeof(double*) * h));
    assert(img->matrice != NULL);

    for(int i = 0; i < h; i++){
        img->matrice[i] = (double*)(malloc(sizeof(double) * l));
        assert(img->matrice[i] != NULL);
    }
    return img;
}


// Fonction qui enregistre une image_s dans un fichier dont le nom est passé en paramètre
void save_image(image* img, char* nom_fichier){
    FILE* fichier = fopen(nom_fichier, "w" );
    assert(fichier != NULL);
	
    fprintf(fichier, "P2\n%d %d\n%d\n", img->longueur, img->hauteur, img->niveau);
    for(int i = 0; i < img->hauteur; i++){
        for(int j = 0; j < img->longueur; j++){
			int tmp = (int)img->matrice[i][j];
            fprintf(fichier, "%d ", tmp);
		}
        fprintf(fichier,"\n");
    }

    fclose(fichier);
}

//prend un nom de fichier d'une image en format pgm et sort un pointeur vers un struct image
image* lecture_image(char* nom_fichier){
    FILE* fichier = fopen(nom_fichier, "r");
    assert(fichier != NULL);

    int l,h,m;
    fscanf(fichier, "P2 %d %d %d", &l, &h, &m);
    printf("l %d h %d m %d\n", l, h, m);
    image* img = create_image(h, l, m);

    for(int i = 0; i < h; i++){
        for(int j = 0; j < l; j++){
            double pixel;
            fscanf(fichier, "%lf", &pixel);
			printf("%lf ", pixel);
            img->matrice[i][j] = pixel;
        }
		printf("\n");
    }

    fclose(fichier);
    return img;
}

// Fonction qui permet de libérer une structure image
void free_image(image* img){
    assert(img != NULL);

    for(int i = 0; i < img->hauteur; i++){
        if(img->matrice[i] != NULL){
            free(img->matrice[i]);
        }
    }

    free(img->matrice);
    free(img);
}

//remplis l'image en noir ou en blanc
void remplissage(double couleur, image* img){
	for(int i = 0 ; i < img->hauteur ; i++){
		for(int j=0 ; j<img->longueur; j++)
			img->matrice[i][j] = couleur;
	}
}

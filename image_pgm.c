// ############ Fonctions de lecture et traitement d'images en format pgm #############

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

// ============ Lecture et enregistrement d'images =============

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
    image* img = create_image(h, l, m);

    for(int i = 0; i < h; i++){
        for(int j = 0; j < l; j++){
            double pixel;
            fscanf(fichier, "%lf", &pixel);
			//printf("%lf ", pixel);
            img->matrice[i][j] = pixel;
        }
    }

    fclose(fichier);
    return img;
}

// Libere une structure image
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

// ========= Manipulation d'images ===========

//prend une image, les coordonné d'un premier pixel (inclu), d'un deuxieme pixel (incluc) et
//renvoit l'image extraite entre les deux pixels
image* extraire_image(int pos1x, int pos1y, int pos2x, int pos2y, image* img){
	int hauteur = pos2x - pos1x + 1;
	assert(hauteur >= 1 && hauteur <= img->hauteur);
	int longueur = pos2y - pos1y + 1;
	assert(longueur >= 1 && longueur <= img->longueur);
	image* resultat = create_image(hauteur, longueur, img->niveau);
    for(int i = 0; i < hauteur; i++){
        for(int j = 0; j < longueur; j++){
			resultat->matrice[i][j] = img->matrice[pos1x + i][pos1y + j];
		}
	}
	return resultat;
}

//prend une image et renvoit la meme image décimé par deux
//Un pixel de l'image renvoyé est la moyenne des 4 pixels corespondant dans l'image de départ
image* decime_image(image* img){
	image* resultat = create_image(img->hauteur / 2, img->longueur / 2, img->niveau);
    for(int i = 0; i < resultat->hauteur; i++){
        for(int j = 0; j < resultat->longueur; j++){
			resultat->matrice[i][j] = (img->matrice[2*i][2*j] + img->matrice[2*i + 1][2*j] + img->matrice[2*i][2*j + 1] + img->matrice[2*i + 1 ][2*j + 1]) / 4;
		}
	}
	return resultat;
}


//remplis l'image de la couleur demandée
void remplissage(double couleur, image* img){
	for(int i = 0 ; i < img->hauteur ; i++){
		for(int j=0 ; j<img->longueur; j++)
			img->matrice[i][j] = couleur;
	}
}

//ajoute une constante a tous les pixel (sans sortir de l'interval [0, niveau])
void add_number_image(double couleur, image* img){
	double tmp;
	for(int i = 0 ; i < img->hauteur ; i++){
		for(int j=0 ; j < img->longueur; j++){
			tmp = img->matrice[i][j] + couleur;
			if(tmp >= 0 && tmp <= img->niveau)	
				img->matrice[i][j] = tmp;
			else{
				if(tmp >= 0)
					img->matrice[i][j] = img->niveau;
				else
					img->matrice[i][j] = 0;
					
			}
		}
	}
}


//multiplie une constante a tous les pixel (sans sortir de l'interval [0, niveau])
void multiply_number_image(double couleur, image* img){
	double tmp;
	for(int i = 0 ; i < img->hauteur ; i++){
		for(int j=0 ; j < img->longueur; j++){
			tmp = img->matrice[i][j] * couleur;
			if(tmp >= 0 && tmp <= img->niveau)	
				img->matrice[i][j] = tmp;
			else{
				if(tmp >= 0)
					img->matrice[i][j] = img->niveau;
				else
					img->matrice[i][j] = 0;
					
			}
		}
	}
}

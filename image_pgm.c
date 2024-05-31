// ############ Fonctions de lecture et traitement d'images en format pgm #############

#ifndef STANDARD
#define STANDARD
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
	printf("Enregistrement en .pgm\n");
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
void extraire_image(int pos1x, int pos1y, int pos2x, int pos2y, image* img, image* resultat){
	int hauteur = pos2x - pos1x + 1;
	assert(hauteur >= 1 && hauteur <= img->hauteur);
	int longueur = pos2y - pos1y + 1;
	assert(longueur >= 1 && longueur <= img->longueur);
	assert(longueur == resultat->longueur && hauteur == resultat->hauteur);
    for(int i = 0; i < hauteur; i++){
        for(int j = 0; j < longueur; j++){
			resultat->matrice[i][j] = img->matrice[pos1x + i][pos1y + j];
		}
	}
}

//prend une image et renvoit la meme image décimé par deux
//Un pixel de l'image renvoyé est la moyenne des 4 pixels corespondant dans l'image de départ
void decime_image(image* img, image* resultat){
	assert(img->longueur == resultat->longueur * 2 && img->hauteur == resultat->hauteur * 2);
    for(int i = 0; i < resultat->hauteur; i++){
        for(int j = 0; j < resultat->longueur; j++){
			resultat->matrice[i][j] = (img->matrice[2*i][2*j] + img->matrice[2*i + 1][2*j] + img->matrice[2*i][2*j + 1] + img->matrice[2*i + 1 ][2*j + 1]) / 4;
		}
	}
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



//s'assure que les pixels de l'image sont entre 0 et niveau pour que l'image soit lisible
void seuil_pixels(image* img){
	for(int i = 0; i < img->hauteur; i++){
		for(int j = 0; j < img->longueur; j++){
			if(img->matrice[i][j] < 0)
				img->matrice[i][j] = 0;
			if(img->matrice[i][j] > img->niveau)
				img->matrice[i][j] = (double)img->niveau;
			
		}
	}	
}

//multiplie une constante a tous les pixel (sans sortir de l'interval [0, niveau])
void multiply_number_image(double couleur, image* img){
	for(int i = 0 ; i < img->hauteur ; i++){
		for(int j=0 ; j < img->longueur; j++){
			img->matrice[i][j] *= couleur;
		}
	}
	
	//seuil_pixels(img);
}

//Isométries du carré : prend une image carré et un identifiant d'isométrie et lui fait subir l'isométrie.
//L'identité est representé par n(importe quel identifiant >= 7 (7 dans le programme)
//Entrée : img l'image, res l'image ou s'applique la transformation, id l'identifiant de la transformation
void isometrie(image* img, image* res, int id){
	assert(img->hauteur == img->longueur && img->longueur == res->hauteur && res->hauteur == res->longueur);
	int n = img->hauteur;
	if(id == 0){
		for(int i = 0 ; i < img->hauteur; i++){
			for(int j=0 ; j < img->longueur; j++){
				res->matrice[i][j] = img->matrice[n-1-j][i];
			}
		}
	}
	if(id == 1){
		for(int i = 0 ; i < img->hauteur; i++){
			for(int j=0 ; j < img->longueur; j++){
				res->matrice[i][j] = img->matrice[n-1-i][n-1-j];
			}
		}
	}
	if(id == 2){
		for(int i = 0 ; i < img->hauteur; i++){
			for(int j=0 ; j < img->longueur; j++){
				res->matrice[i][j] = img->matrice[j][n-1-i];
			}
		}
	}
	if(id == 3){
		for(int i = 0 ; i < img->hauteur; i++){
			for(int j=0 ; j < img->longueur; j++){
				res->matrice[i][j] = img->matrice[n-1-i][j];
			}
		}
	}
	if(id == 4){
		for(int i = 0 ; i < img->hauteur; i++){
			for(int j=0 ; j < img->longueur; j++){
				res->matrice[i][j] = img->matrice[j][i];
			}
		}
	}
	if(id == 5){
		for(int i = 0 ; i < img->hauteur; i++){
			for(int j=0 ; j < img->longueur; j++){
				res->matrice[i][j] = img->matrice[i][n-1-j];
			}
		}
	}
	if(id == 6){
		for(int i = 0 ; i < img->hauteur; i++){
			for(int j=0 ; j < img->longueur; j++){
				res->matrice[i][j] = img->matrice[n-1-j][n-1-i];
			}
		}
	}
	if(id == 7){
		for(int i = 0 ; i < img->hauteur; i++){
			for(int j=0 ; j < img->longueur; j++){
				res->matrice[i][j] = img->matrice[i][j];
			}
		}
	}
}


//prend deux image et revoie la différence des deux, centré sur le niveau de gris médiant.

image* difference(image * img1, image* img2){
	assert(img1->hauteur == img2-> hauteur && img1->longueur == img2->longueur);
	assert(img1-> niveau = img2->niveau);
	int h = img1->hauteur;
	int l = img1->longueur;
	int n = img1 ->niveau;

	image* res = create_image(h,l,n);
	remplissage(n/2, res);

	for(int i = 0; i< h; i++){
		for(int j = 0; j < l; j++){
			res->matrice[i][j] += img1->matrice[i][j];
			res->matrice[i][j] -= img2->matrice[i][j];
		}
	}

	seuil_pixels(res);

	return res;
}

image* erreur(image * img1, image* img2){
	assert(img1->hauteur == img2-> hauteur && img1->longueur == img2->longueur);
	assert(img1-> niveau = img2->niveau);
	int h = img1->hauteur;
	int l = img1->longueur;
	int n = img1 ->niveau;

	image* res = create_image(h,l,n);
	remplissage(n/2, res);

	for(int i = 0; i< h; i++){
		for(int j = 0; j < l; j++){
			res->matrice[i][j] = (img1->matrice[i][j] - img2->matrice[i][j]) * (img1->matrice[i][j] - img2->matrice[i][j]);
		}
	}

	seuil_pixels(res);

	return res;
}





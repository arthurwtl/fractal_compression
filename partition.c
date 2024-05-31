// ################### Blocs et Partitions #####################


#ifndef LECTURE
#define LECTURE
#include "image_pgm.c"
#endif


// un bloc est la donné de : une image, les constantes de la ragression linéaire, les coordonnés du bloc source.
struct bloc_s{
	image* img;
	double alpha;
	double beta;
	double cout;
	int posx;
	int posy;
	int id_iso;
};

typedef struct bloc_s bloc;


bloc* create_bloc(image* img){
    bloc* b = (bloc*)malloc(sizeof(bloc));
    assert(b != NULL);

	b->img = img;
	b->alpha = -1;
	b->beta = -1;
	b->cout = -1;
	b->posx = -1;
	b->posy = -1;
	b->id_iso = -1;

	return b;
}

void free_bloc(bloc* b){
	free_image(b->img);
	free(b);
}

/* Une partition est une matrice de bloc de taille pas * pas. Les blocs des extrimités sont autorisé a etre rectangulaires
 * Une partion d'une image contient toutes les données de l'image*/
struct partition_s{
	int pas; 
	
	//les information suivantes sont redondante mais facilitent le code
	int hauteur;
	int longueur;
	int niveau;

	int nbBlocHauteur;
	int nbBlocLongueur;
	
	bloc*** blocs;
};

typedef struct partition_s partition;

// ============ Image to part et part to image ==============

partition* partition_from_image(image* img, int pas){
	partition* part = malloc(sizeof(partition));
	assert(part != NULL);


	part->hauteur = img->hauteur;
	part->longueur = img->longueur;
	part->niveau = img->niveau;
	part->pas = pas;

	int nbBlocHauteur = part->hauteur / pas;
	int rh = part->hauteur % pas; 
	/*soit rh != 0, dans ce cas il y a un bloc de plus, de hauteur rh
	 *soit rh == 0, dans ce cas la hauteur du dernier bloc est pas, doncon pose rh = pas pour la suite*/
	if(rh != 0)
		nbBlocHauteur++;
	else
		rh = pas;
	part->nbBlocHauteur = nbBlocHauteur;

	int nbBlocLongueur = part->longueur / pas;
	int rl = part->longueur % pas; 
	if(rl != 0)
		nbBlocLongueur++;
	else
		rl = pas;
	part->nbBlocLongueur = nbBlocLongueur;

	printf("Partition crée\n");
	printf("nb bloc hauteur : %d\nnb bloc longueur : %d\nniveau : %d\npas : %d\nrh : %d\nrl : %d\n", nbBlocHauteur, nbBlocLongueur, part->niveau, pas, rh, rl);


	part->blocs = (bloc***)malloc(sizeof(bloc**) * nbBlocHauteur);
	assert(part->blocs != NULL);
	
	for(int i = 0; i < nbBlocHauteur; i++){
		part->blocs[i] = (bloc**)malloc(sizeof(bloc*) * nbBlocLongueur);
		assert(part->blocs[i] != NULL);

		for(int j = 0; j < nbBlocLongueur; j++){
			
			if(i < nbBlocHauteur - 1 && j < nbBlocLongueur - 1){
				printf("0");
			}
			if(i < nbBlocHauteur - 1 && j == nbBlocLongueur - 1){
				printf("1\n");
			}
			if(i == nbBlocHauteur - 1 && j < nbBlocLongueur - 1){
				printf("2");
			}
			if(i == nbBlocHauteur - 1 && j == nbBlocLongueur - 1){
				printf("4\n");
			}

		}
		
		for(int j = 0; j < nbBlocLongueur; j++){
			if(i < nbBlocHauteur - 1 && j < nbBlocLongueur - 1){
				image* tmp = create_image(pas, pas, img->niveau);
				extraire_image(i*pas, j*pas, (i+1)*pas - 1, (j+1)*pas - 1, img, tmp);
				part->blocs[i][j] = create_bloc(tmp);
			}
			if(i < nbBlocHauteur - 1 && j == nbBlocLongueur - 1){
				image* tmp = create_image(pas, rl, img->niveau);
				extraire_image(i*pas, j*pas, (i+1)*pas - 1, j*pas + rl - 1, img, tmp);
				part->blocs[i][j] = create_bloc(tmp);
			}
			if(i == nbBlocHauteur - 1 && j < nbBlocLongueur - 1){
				image* tmp = create_image(rh, pas, img->niveau);
				extraire_image(i*pas, j*pas, i*pas + rh - 1, (j+1)*pas - 1, img, tmp);
				part->blocs[i][j] = create_bloc(tmp);
			}
			if(i == nbBlocHauteur - 1 && j == nbBlocLongueur - 1){
				image* tmp = create_image(rh, rl, img->niveau);
				extraire_image(i*pas, j*pas, i*pas + rh - 1, j*pas + rl - 1, img, tmp);
				part->blocs[i][j] = create_bloc(tmp);
			}

		}
	}
	printf("Partition bien crée\n");
	return part;	
}


void free_partition(partition* part){
	for(int i = 0; i < part->nbBlocHauteur; i++){
		for(int j = 0; j < part->nbBlocLongueur; j++){
			free_bloc(part->blocs[i][j]);
		}
		free(part->blocs[i]);
	}
	free(part->blocs);
	free(part);
}

/*s'assure que les pixels de l'image sont entre 0 et niveau pour que l'image soit lisible deplacé dans images
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
*/

image* image_from_partition(partition* part){
	image* resultat = create_image(part->hauteur, part->longueur, part->niveau);

	for(int i = 0; i < part->nbBlocHauteur; i++){
		for(int j = 0; j < part->nbBlocLongueur; j++){
			for(int k = 0; k < part->blocs[i][j]->img->hauteur; k++){
				for(int l = 0; l < part->blocs[i][j]->img->longueur; l++){
					resultat->matrice[part->pas * i + k][part->pas * j + l] = part->blocs[i][j]->img->matrice[k][l];
				}
			}
		}
	}
			
	//on seuil les pixels entre 0 et niveau
	seuil_pixels(resultat);
	return resultat;
}

// auxiliere pour la fonction suivante
void contour_bloc(bloc* b){
	for(int i = 0; i < b->img->hauteur; i++)
		b->img->matrice[i][0] = 0;

	for(int i = 0; i < b->img->longueur; i++)
		b->img->matrice[0][i] = 0;
}

//crée une grille sur l'image de la partition
void grille(partition* part){
	for(int i = 0; i < part->nbBlocHauteur; i++){
		for(int j = 0; j < part->nbBlocLongueur; j++){
			contour_bloc(part->blocs[i][j]);
		}
	}
}

// ============== Enregistrement et lecture du fichier compressé en .ftl ==============

//enregistre une image sous forme compressé : format detaillé dans fractale.txt
void save_fractale(partition* part, char* nom_fichier){
	printf("Enregistrement en .ftl\n");
    FILE* fichier = fopen(nom_fichier, "w" );
    assert(fichier != NULL);
	
    fprintf(fichier, "%d\n%d\n%d\n%d\n", part->longueur, part->hauteur, part->niveau, part->pas);

	for(int i = 0; i < part->nbBlocHauteur; i++){
		for(int j = 0; j < part->nbBlocLongueur; j++){
			fprintf(fichier, "%lf\n%lf\n%d\n%d\n%d\n", part->blocs[i][j]->alpha, part->blocs[i][j]->beta, part->blocs[i][j]->posx, part->blocs[i][j]->posy, part->blocs[i][j]->id_iso);
		}
	}

    fclose(fichier);
}

//Lit une image .ftl et la stock dans une partition dont on ne se préoccupe pas de l'image
partition* lecture_fractale(char* nom_fichier){

	printf("lecture de %s :\n", nom_fichier);
    FILE* fichier = fopen(nom_fichier, "r");
    assert(fichier != NULL);

	int longueur, hauteur, niveau, pas, posx, posy, id_iso;
	double alpha, beta;

    fscanf(fichier, "%d\n%d\n%d\n%d\n", &longueur, &hauteur, &niveau, &pas);
    //printf("l %d h %d n %d p %d\n", longueur, hauteur, niveau, pas);

	image* img_tmp = create_image(hauteur , longueur , niveau);
	partition* part = partition_from_image(img_tmp, pas);
	free_image(img_tmp);

	for(int i = 0; i < part->nbBlocHauteur; i++){
		for(int j = 0; j < part->nbBlocLongueur; j++){
			fscanf(fichier, "%lf\n%lf\n%d\n%d\n%d\n", &alpha, &beta, &posx, &posy, &id_iso);
			//printf("a %lf b %lf x %d y %d\n", alpha, beta, posx, posy);
			part->blocs[i][j]->alpha = alpha;
			part->blocs[i][j]->beta = beta;
			part->blocs[i][j]->posx = posx;
			part->blocs[i][j]->posy = posy;
			part->blocs[i][j]->id_iso = id_iso;
		}
	}
    fclose(fichier);
	return part;
}



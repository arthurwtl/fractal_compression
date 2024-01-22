#ifndef LECTURE
#define LECTURE
#include "image_pgm.c"
#endif

// =================== Blocs et Partitions =====================

// un bloc est la donné de : une image, les constantes de la ragression linéaire, les coordonnés du bloc source.
struct bloc_s{
	image* img;
	double alpha;
	double beta;
	double cout;
	int posx;
	int posy;
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

	printf("nb bloc hauteur : %d\nnb bloc longueur : %d\nniveau : %d\npas : %d\nrh : %d\n rl : %d\n", nbBlocHauteur, nbBlocLongueur, part->niveau, pas, rh, rl);


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
				part->blocs[i][j] = create_bloc(extraire_image(i*pas, j*pas, (i+1)*pas - 1, (j+1)*pas - 1, img));
			}
			if(i < nbBlocHauteur - 1 && j == nbBlocLongueur - 1){
				part->blocs[i][j] = create_bloc(extraire_image(i*pas, j*pas, (i+1)*pas - 1, j*pas + rl - 1, img));
			}
			if(i == nbBlocHauteur - 1 && j < nbBlocLongueur - 1){
			
				part->blocs[i][j] = create_bloc(extraire_image(i*pas, j*pas, i*pas + rh - 1, (j+1)*pas - 1, img));
			}
			if(i == nbBlocHauteur - 1 && j == nbBlocLongueur - 1){
				part->blocs[i][j] = create_bloc(extraire_image(i*pas, j*pas, i*pas + rh - 1, j*pas + rl - 1, img));
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

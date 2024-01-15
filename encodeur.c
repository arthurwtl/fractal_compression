// ################################## Compression fractale ##########################################



/* Les fonction de lecture et traitement d'images sont dans image_pgm.c
 *
 * Ici : on prend une image. On la découpe en bloc de taille 8*8 dans une matrice appelé partition. On applique l'algorithme */

#include "image_pgm.c"


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

// ============= Regression linéaire =============


//renvoit la somme des pixels de l'image ou chaque pixel et le produit des pixels de meme coordonnées dans les images données en parametre
double somme_du_produit_images(image* img1, image* img2){
	assert(img1->hauteur == img2->hauteur && img1->longueur == img2->longueur && img1->niveau == img2->niveau);
	
	int resultat = 0;

    for(int i = 0; i < img1->hauteur; i++){
        for(int j = 0; j < img1->longueur; j++){
			resultat += img1->matrice[i][j] * img2->matrice[i][j];
		}
	}
	return resultat;
}


//prend une image et renvoit la somme de ses pixels
double somme_image(image* img){
	int res = 0;

    for(int i = 0; i < img->hauteur; i++){
        for(int j = 0; j < img->longueur; j++){
			res += img->matrice[i][j];
		}
	}
	return res;
}


//somme des carrés de la difference des pixels
double cout(image* img1, image* img2){
	assert(img1->hauteur == img2->hauteur && img1->longueur == img2->longueur && img1->niveau == img2->niveau);
	
	int resultat = 0;

    for(int i = 0; i < img1->hauteur; i++){
        for(int j = 0; j < img1->longueur; j++){
			resultat += (img1->matrice[i][j] - img2->matrice[i][j]) * (img1->matrice[i][j] - img2->matrice[i][j]);;
		}
	}
	return resultat;
}


/*Prend un bloc et une image de la meme taille  que l'image du bloc. Si la regression linéaire entre les deux donne un cout meilleur que ce qui
 * est enregistré dans le bloc, ses variables sont actualisées.*/
void regression_lineaire(bloc* b, image* img, int posx, int posy){
	double sommeUV = somme_du_produit_images(b->img, img);
	double sommeU = somme_image(b->img);
	double sommeV = somme_image(img);
	double sommeVcarre = somme_du_produit_images(b->img, b->img); 
	double N = b->img->longueur * b->img->hauteur;

	double alpha = (sommeUV * N - sommeU * sommeV) / (sommeVcarre * N - sommeV * sommeV);
	double beta = (alpha * sommeV - sommeU) / N;

	double cout = 0;
    for(int i = 0; i < b->img->hauteur; i++){
        for(int j = 0; j < b->img->longueur; j++){
			cout += (b->img->matrice[i][j] - alpha * img->matrice[i][j] + beta) * (b->img->matrice[i][j] - alpha * img->matrice[i][j] + beta);
		}
	}

	if(b->posx == -1 || cout < b->cout){
		b->alpha = alpha;
		b->beta = beta;
		b->cout = cout;
		b->posx = posx;
		b->posy = posy;
	}
}

// =================== L'algo =====================

//prend une image, un pas pour la partition et un pas pour la precision de la recherche des blocs source
//Sort une partition avec tout actualisé
partition* compression(image* img, int pasPartition, int pasRecherche){
	partition* part = partition_from_image(img, pasPartition);

	int posx = 0;
	int posy = 0;

	for(int i = 0; i < part->nbBlocHauteur; i++){
		for(int j = 0; j < part->nbBlocLongueur; j++){
			while(posx < img->hauteur - 2 * part->blocs[i][j]->img->hauteur){
				while(posy < img->longueur - 2 * part->blocs[i][j]->img->longueur){
					image* tmp1 = extraire_image(posx, posy, posx + 2 * part->blocs[i][j]->img->hauteur - 1, posy + 2 * part->blocs[i][j]->img->longueur - 1, img);
					printf("bloc i=%d, j=%d, posx %d, posy %d, hauteur %d, longueur %d\n", i, j, posx, posy, part->blocs[i][j]->img->hauteur, part->blocs[i][j]->img->longueur);
					image* tmp2 = decime_image(tmp1);
					regression_lineaire(part->blocs[i][j], tmp2, posx, posy);	
					free_image(tmp1);
					free_image(tmp2);

					posy += pasRecherche;
				}
				posx += pasRecherche;
				posy = 0;
			}
			posx = 0;
		}
	}
		
	return part;
}

//auxiliere uniquement pour la fonction suivante, qui est d'ailleur codé avec un pied
double aux(double val, double alpha, double beta, int niveau){
	double tmp = alpha * val + beta;
	double res = -1;
	if(tmp >= 0 && tmp <= niveau)	
		res = tmp;
	else{
		if(tmp >= 0)
			res = niveau;
		else
			res = 0;
	}
	return res;
}

//une itération du processus de décompression
void une_iteration(partition* part, image* source, image* resultat){
	double tmp;
	for(int i = 0; i < part->nbBlocHauteur; i++){
		for(int j = 0; j < part->nbBlocLongueur; j++){
			int posx = part->blocs[i][j]->posx;
			int posy = part->blocs[i][j]->posy;
			for(int k = 0; k < part->blocs[i][j]->img->hauteur; k++){
				for(int l = 0; l < part->blocs[i][j]->img->longueur; l++){
					tmp  = (source->matrice[posx + 2*k][posy + 2*l] + source->matrice[posx + 2*k + 1][ posy + 2*l] + source->matrice[ posx +2*k][posy + 2*l + 1] + source->matrice[posx + 2*k + 1 ][posy + 2*l + 1]) / 4;
					resultat->matrice[i * part->pas + k][j * part->pas + l] = aux(tmp, part->blocs[i][j]->alpha, part->blocs[i][j]->beta, resultat->niveau);
				}
			}

		}
	}
}

image* decompression(partition* part, int n){
	image* img1 = create_image(part->hauteur, part->longueur, part->niveau);
	image* img2 = create_image(part->hauteur, part->longueur, part->niveau);

	remplissage(part->niveau/2, img1);
	remplissage(part->niveau/2, img2);

	for(int i = 0; i < n/2; i++){
		une_iteration(part, img1, img2);
		une_iteration(part, img2, img1);
	}
	if(n % 2 == 1){
		une_iteration(part, img1, img2);
		free_image(img1);
		return img2;	
	}
	else{
		free_image(img2);
		return img1;
	}
}

int main()
{
	// ================== Tests divers =================
	
	//test creation, sauvegarde, free et remplissage
	image* test1 = create_image(100, 80, 255);
	remplissage(100, test1);
	save_image(test1, "test1.pgm");

	//test lecture
	image* test2 = lecture_image("feep.pgm");
	save_image(test2, "test2.pgm");

	//test extraction images
	image* test3 = extraire_image(0, 0, test2->hauteur - 1, test2->longueur - 1, test2);
	save_image(test3, "test3.pgm");

	printf(" les images on un score de : %d\n", (int)cout(test3, test3));


	//test decimation
	image* mangues = lecture_image("mangues.pgm");
	image* manguesExpose = lecture_image("manguesExpose.pgm");
	image* test4 = decime_image(mangues);
	save_image(test4, "test4.pgm");

	bloc* mangues_b = create_bloc(mangues);

	regression_lineaire(mangues_b, manguesExpose, 0, 0);
	printf("alpha : %lf beta : %lf cout : %lf posx : %d posy : %d\n",mangues_b->alpha, mangues_b->beta, mangues_b->cout, mangues_b->posx, mangues_b->posy);

	multiply_number_image(mangues_b->alpha, manguesExpose);
	add_number_image(mangues_b->beta, manguesExpose);

	save_image(manguesExpose, "resultat_test_reg_lin.pgm");

	partition* mangues_partition = compression(test1, 8, 4);
	grille(mangues_partition);
	printf("%d\n", mangues_partition->blocs[12][9]->img->hauteur); //c'est les blocs de frontiere qui sont cassés

	image* test5 = image_from_partition(mangues_partition);
	
	save_image(test5, "test5.pgm");

	free_image(test5);
	free_partition(mangues_partition);
	free(mangues_b);
	free_image(mangues);
	free_image(manguesExpose);
	free_image(test1);	
	free_image(test2);
	free_image(test3);
	free_image(test4);

	// ================== tests de l'algo ===================
	
	image* fruits = lecture_image("mangues.pgm");

	partition* fruits_partition = compression(fruits, 32, 16);
	double tmp = fruits_partition->blocs[2][0]->beta;
	printf("tmp : %lf\n", tmp);

	image* resultat = decompression(fruits_partition, 1);
	//une_iteration(fruits_partition, img1, img2);

	save_image(resultat, "decompressé.pgm");

	free_partition(fruits_partition);
	free_image(resultat);
	free_image(fruits);



	return 0;
}

// ################################## Compression fractale ##########################################



/* Les fonction de lecture et traitement d'images sont dans image_pgm.c
 *
 * Ici : on prend une image. On la découpe en bloc de taille 8*8 dans une matrice appelé partition. On applique l'algorithme */

#ifndef LECTURE
#define LECTURE
#include "image_pgm.c"
#endif

#ifndef PARTITION
#define PARTITION
#include "partition.c"
#endif

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
void regression_lineaire(bloc* b, image* img, int posx, int posy, int id_iso){
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
		b->id_iso = id_iso;
	}
}

// =================== L'algo =====================

//prend une image, un pas pour la partition et un pas pour la precision de la recherche des blocs source
//Sort une partition avec tout actualisé
partition* compression(image* img, int pasPartition, int pasRecherche){
	
	printf("Début de compression :\n");
	partition* part = partition_from_image(img, pasPartition);

	int posx;
	int posy;

	for(int i = 0; i < part->nbBlocHauteur; i++){
		printf("Compression %d sur %d\n", i, part->nbBlocHauteur);
		for(int j = 0; j < part->nbBlocLongueur; j++){
			if(part->blocs[i][j]->img->longueur == part->blocs[i][j]->img->hauteur){
				image* image_extraite = create_image(part->blocs[i][j]->img->hauteur * 2, part->blocs[i][j]->img->longueur * 2, part->blocs[i][j]->img->niveau);
				image* image_decime = create_image(part->blocs[i][j]->img->hauteur, part->blocs[i][j]->img->longueur, part->blocs[i][j]->img->niveau);
				image* image_iso = create_image(part->blocs[i][j]->img->hauteur, part->blocs[i][j]->img->longueur, part->blocs[i][j]->img->niveau);
				posx = 0;
				posy = 0;
				while(posx < img->hauteur - 2 * part->blocs[i][j]->img->hauteur){
					while(posy < img->longueur - 2 * part->blocs[i][j]->img->longueur){
						extraire_image(posx, posy, posx + 2 * part->blocs[i][j]->img->hauteur - 1, posy + 2 * part->blocs[i][j]->img->longueur - 1, img, image_extraite);
						//printf("bloc i=%d, j=%d, posx %d, posy %d, hauteur %d, longueur %d\n", i, j, posx, posy, part->blocs[i][j]->img->hauteur, part->blocs[i][j]->img->longueur);
						decime_image(image_extraite, image_decime);
						
						for(int id_iso = 0; id_iso <= 7; id_iso++){
							isometrie(image_decime, image_iso, id_iso);
							regression_lineaire(part->blocs[i][j], image_iso, posx, posy, id_iso);	
						}

						posy += pasRecherche;
					}
					posx += pasRecherche;
					posy = 0;
				}
				free_image(image_decime);
				free_image(image_extraite);
				free_image(image_iso);
			}
			else{
				image* image_extraite = create_image(part->blocs[i][j]->img->hauteur * 2, part->blocs[i][j]->img->longueur * 2, part->blocs[i][j]->img->niveau);
				image* image_decime = create_image(part->blocs[i][j]->img->hauteur, part->blocs[i][j]->img->longueur, part->blocs[i][j]->img->niveau);
				posx = 0;
				posy = 0;
				while(posx < img->hauteur - 2 * part->blocs[i][j]->img->hauteur){
					while(posy < img->longueur - 2 * part->blocs[i][j]->img->longueur){
						extraire_image(posx, posy, posx + 2 * part->blocs[i][j]->img->hauteur - 1, posy + 2 * part->blocs[i][j]->img->longueur - 1, img, image_extraite);
						//printf("bloc i=%d, j=%d, posx %d, posy %d, hauteur %d, longueur %d\n", i, j, posx, posy, part->blocs[i][j]->img->hauteur, part->blocs[i][j]->img->longueur);
						decime_image(image_extraite, image_decime);

						regression_lineaire(part->blocs[i][j], image_decime, posx, posy, 7);	

						posy += pasRecherche;
					}
					posx += pasRecherche;
					posy = 0;
				}
				free_image(image_decime);
				free_image(image_extraite);
			}
		}
	}

	printf("Fin de compression :\n");
	return part;
}

//une itération du processus de décompression
void une_iteration(partition* part, image* source, image* resultat){
	double val_pixel;
	for(int i = 0; i < part->nbBlocHauteur; i++){
		for(int j = 0; j < part->nbBlocLongueur; j++){
			if(part->blocs[i][j]->img->longueur == part->blocs[i][j]->img->hauteur){
				int posx = part->blocs[i][j]->posx;
				int posy = part->blocs[i][j]->posy;
				image* image_extraite = create_image(part->blocs[i][j]->img->hauteur * 2, part->blocs[i][j]->img->longueur * 2, part->blocs[i][j]->img->niveau);
				image* image_decime = create_image(part->blocs[i][j]->img->hauteur, part->blocs[i][j]->img->longueur, part->blocs[i][j]->img->niveau);
				extraire_image(posx, posy, posx + 2 * part->blocs[i][j]->img->hauteur - 1, posy + 2 * part->blocs[i][j]->img->longueur - 1, source, image_extraite);
				decime_image(image_extraite, image_decime);
				isometrie(image_decime, part->blocs[i][j]->img, part->blocs[i][j]->id_iso);
				for(int k = 0; k < part->blocs[i][j]->img->hauteur; k++){
					for(int l = 0; l < part->blocs[i][j]->img->longueur; l++){
						resultat->matrice[i * part->pas + k][j * part->pas + l] = part->blocs[i][j]->img->matrice[k][l] * part->blocs[i][j]->alpha - part->blocs[i][j]->beta;
					}
				}
				free_image(image_decime);
				free_image(image_extraite);
			}
			else{
				int posx = part->blocs[i][j]->posx;
				int posy = part->blocs[i][j]->posy;
				for(int k = 0; k < part->blocs[i][j]->img->hauteur; k++){
					for(int l = 0; l < part->blocs[i][j]->img->longueur; l++){
						val_pixel  = (source->matrice[posx + 2*k][posy + 2*l] + source->matrice[posx + 2*k + 1][ posy + 2*l] + source->matrice[ posx +2*k][posy + 2*l + 1] + source->matrice[posx + 2*k + 1 ][posy + 2*l + 1]) / 4;
						resultat->matrice[i * part->pas + k][j * part->pas + l] = val_pixel * part->blocs[i][j]->alpha - part->blocs[i][j]->beta;
					}
				}
			}

		}
	}
}

image* decompression(partition* part, int n){
	printf("Début de décompression :\n");
	image* img1 = create_image(part->hauteur, part->longueur, part->niveau);
	image* img2 = create_image(part->hauteur, part->longueur, part->niveau);

	remplissage(part->niveau/2, img1);
	remplissage(part->niveau/2, img2);

	for(int i = 0; i < n/2; i++){
		printf("Decompression %d sur %d\n", i*2, n);
		une_iteration(part, img1, img2);
		une_iteration(part, img2, img1);
	}
	if(n % 2 == 1){
		une_iteration(part, img1, img2);
		free_image(img1);
		seuil_pixels(img2);
		printf("Fin de décompression :\n");
		return img2;	
	}
	else{
		free_image(img2);
		seuil_pixels(img1);
		printf("Fin de décompression :\n");
		return img1;
	}
}

int main()
{
	// ================== Tests divers =================

	/*
	//Génere les image de F caracterisant les isométries
	image* test1 = lecture_image("feep.pgm");
	save_image(test1, "test1.pgm");

	image* test2 = extraire_image(0, 0, test1->hauteur - 1, test1->hauteur - 1, test1);
	save_image(test2, "test2.pgm");

	image* test3 = create_image(test2->hauteur, test2->hauteur, test2->niveau);
	for(int i = 0; i <= 7; i++){
		isometrie(test2, test3, i);
		char* name = malloc(256);
		strcpy(name, "isometrie_nb_x.pgm");
		name[13] = i+48;
		printf("%c\n", name[13]);
		printf("%s\n", name);

		save_image(test3, name);
		free(name);
	}

	free_image(test1);
	free_image(test2);
	free_image(test3);
	*/


	// ================== tests de l'algo ===================
	
	printf("\n========= Execution de l'algorithme =========\n\n");
	
	//image* image_non_compresse = lecture_image("mangues.pgm");

	//partition* partition_compresse = compression(image_non_compresse, 16, 8);
	//save_fractale(partition_compresse, "fractale.ftl");

	partition* partition_compresse = lecture_fractale("fractale.ftl");
	image* resultat_decompression = decompression(partition_compresse, 50);

	save_image(resultat_decompression, "decompressé.pgm");

	free_partition(partition_compresse);
	free_image(resultat_decompression);
	//free_image(image_non_compresse);



	return 0;
}


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
					//printf("bloc i=%d, j=%d, posx %d, posy %d, hauteur %d, longueur %d\n", i, j, posx, posy, part->blocs[i][j]->img->hauteur, part->blocs[i][j]->img->longueur);
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
		printf("%d sur %d\n", i, part->nbBlocHauteur);
	}
		
	return part;
}

//une itération du processus de décompression
void une_iteration(partition* part, image* source, image* resultat){
	double val_pixel;
	for(int i = 0; i < part->nbBlocHauteur; i++){
		for(int j = 0; j < part->nbBlocLongueur; j++){
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
		seuil_pixels(img2);
		return img2;	
	}
	else{
		free_image(img2);
		seuil_pixels(img1);
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

	partition* fruits_partition = compression(fruits, 32, 2);
	double tmp = fruits_partition->blocs[2][0]->beta;
	printf("tmp : %lf\n", tmp);

	image* resultat = decompression(fruits_partition, 100);
	//une_iteration(fruits_partition, img1, img2);

	save_image(resultat, "decompressé.pgm");

	free_partition(fruits_partition);
	free_image(resultat);
	free_image(fruits);



	return 0;
}


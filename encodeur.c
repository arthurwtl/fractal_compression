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

#include <pthread.h>
#define NUM_THREADS 12

// ============= Regression linéaire =============


//renvoit la somme des pixels de l'image ou chaque pixel et le produit des pixels de meme
//coordonnées dans les images données en parametre
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
long double cout_images(image* img1, image* img2){
	assert(img1->hauteur == img2->hauteur && img1->longueur == img2->longueur && img1->niveau == img2->niveau);
	
	long double resultat = 0;

    for(int i = 0; i < img1->hauteur; i++){
        for(int j = 0; j < img1->longueur; j++){
			resultat += (img1->matrice[i][j] - img2->matrice[i][j]) * (img1->matrice[i][j] - img2->matrice[i][j]);;
		}
	}
	return sqrtl(resultat);
}


/*Prend un bloc et une image de la meme taille  que l'image du bloc. Si la regression linéaire entre les deux donne un cout meilleur que ce qui
 * est enregistré dans le bloc, ses variables sont actualisées.*/
void regression_lineaire(bloc* b, image* img, int posx, int posy, int id_iso){
	double sommeUV = somme_du_produit_images(b->img, img);
	double sommeU = somme_image(b->img);
	double sommeV = somme_image(img);
	double sommeVcarre = somme_du_produit_images(img, img); 
	double N = b->img->longueur * b->img->hauteur;

	double alpha = (sommeUV * N - sommeU * sommeV) / (sommeVcarre * N - sommeV * sommeV + 1);
	//on rajoute un au dénominateur pour éviter les NaN
	double beta = (alpha * sommeV - sommeU) / N;

	double cout = 0;
    for(int i = 0; i < b->img->hauteur; i++){
        for(int j = 0; j < b->img->longueur; j++){
			cout += (b->img->matrice[i][j] - alpha * img->matrice[i][j] + beta) * (b->img->matrice[i][j] - alpha * img->matrice[i][j] + beta);
		}
	}

	if(b->id_iso == -1 || cout <= b->cout){ //id_iso = -1 que lorsque on execute le 1er passage
		b->alpha = alpha;
		b->beta = beta;
		b->cout = cout;
		b->posx = posx;
		b->posy = posy;
		b->id_iso = id_iso;
	}
}

// =================== L'algo =====================

// Structure pour les arguments du thread
struct ThreadArgs {
    partition* part;
    image* img;
    int pasRecherche;
    int start;
    int end;
};
typedef struct ThreadArgs threadArgs;

// Fonction exécutée par chaque thread
void *compress_thread(void *arg) {
    threadArgs *args = (threadArgs *)arg;

	// 
    int pasRecherche = args->pasRecherche;
    int start = args->start;
    int end = args->end;
	partition* part = args->part;
	image* img = args->img;

	int posx;
	int posy;

	int h;
	int l;
	int n;

    // Boucle externe parallélisée
    for(int i = start; i < end; i++) {
		printf("Compression %d sur %d sur le thread %d\n", i-start, end - start, args->start);
        for(int j = 0; j < part->nbBlocLongueur; j++) {
			h = part->blocs[i][j]->img->hauteur;
			l = part->blocs[i][j]->img->longueur;
			n = part->blocs[i][j]->img->niveau;

			if(l == h){
				image* image_extraite = create_image(h * 2, l * 2, n);
				image* image_decime = create_image(h, l, n);
				image* image_iso = create_image(h, l, n);
				posx = 0;
				posy = 0;
				while(posx < img->hauteur - 2 * h){
					while(posy < img->longueur - 2 * l){
						extraire_image(posx, posy, posx + 2 * h - 1, posy + 2 * l - 1, img, image_extraite);
						//printf("bloc i=%d, j=%d, posx %d, posy %d, hauteur %d, longueur %d\n", i, j, posx, posy, h, l);
						decime_image(image_extraite, image_decime);
						
						for(int id_iso = 0; id_iso <= 7; id_iso++){
							isometrie(image_decime, image_iso, id_iso);
							// /!\ remettre id_iso à la place de 7 pour avoir les isométries
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
				image* image_extraite = create_image(h * 2, l * 2, n);
				image* image_decime = create_image(h, l, n);
				posx = 0;
				posy = 0;
				while(posx < img->hauteur - 2 * h){
					while(posy < img->longueur - 2 * l){
						extraire_image(posx, posy, posx + 2 * h - 1, posy + 2 * l - 1, img, image_extraite);
						//printf("bloc i=%d, j=%d, posx %d, posy %d, hauteur %d, longueur %d\n", i, j, posx, posy, h, l);
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

    pthread_exit(NULL);
}

//prend une image, un pas pour la partition et un pas pour la precision de la recherche des blocs source
//Sort une partition avec tout actualisé
partition* compression(image* img, int pasPartition, int pasRecherche) {
    printf("Début de compression :\n");
    partition* part = partition_from_image(img, pasPartition);

    // Création des threads
    pthread_t threads[NUM_THREADS];
    threadArgs thread_args[NUM_THREADS];
    double step = (double)part->nbBlocHauteur / NUM_THREADS;
	
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].part = part;
        thread_args[i].img = img;
        thread_args[i].pasRecherche = pasRecherche;
        thread_args[i].start = (i == 0) ? 0 : thread_args[i-1].end;
        thread_args[i].end = (i == NUM_THREADS - 1) ? part->nbBlocHauteur : (int)((double)(i + 1) * step);
        pthread_create(&threads[i], NULL, compress_thread, (void *)&thread_args[i]);
    }

    // Attente de la fin de tous les threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
	printf("fin de compression\n");

    return part;
}

int max(int a,int b){
	if(a < b)
		return b;
	else
		return a;
}

void *recherche_adjacente_thread(void *arg) {
    threadArgs *args = (threadArgs *)arg;

    int pas = args->pasRecherche;
    int start = args->start;
    int end = args->end;
	partition* part = args->part;
	image* img = args->img;

	int h;
	int l;
	int n;

	for(int i = start; i < end; i++){
		printf("Recherche adjacente %d sur %d sur le thread %d\n", i-start, end - start, args->start / NUM_THREADS);
		for(int j = 0; j < part->nbBlocLongueur; j++){

			h = part->blocs[i][j]->img->hauteur;
			l = part->blocs[i][j]->img->longueur;
			n = part->blocs[i][j]->img->niveau;

			int posx_initial = part->blocs[i][j]->posx;
			int posy_initial = part->blocs[i][j]->posy;
			int posx = max(0, posx_initial - pas);
			int posy = max(0, posy_initial - pas);

			image* image_extraite = create_image(h * 2, l * 2, n);
			image* image_decime = create_image(h, l, n);
			image* image_iso = create_image(h, l, n);

			while(posx < part->hauteur - 2 * h && posx < posx_initial + pas){
				while(posy < part->longueur - 2 * l && posy < posy_initial + pas){
					extraire_image(posx, posy, posx + 2 * h - 1, posy + 2 * l - 1, img, image_extraite);
					//printf("bloc i=%d, j=%d, posx %d, posy %d, hauteur %d, longueur %d\n", i, j, posx, posy, h, l);
					decime_image(image_extraite, image_decime);
				
					if(part->blocs[i][j]->id_iso != 7){
						isometrie(image_decime, image_iso, part->blocs[i][j]->id_iso);
						regression_lineaire(part->blocs[i][j], image_iso, posx, posy, part->blocs[i][j]->id_iso);	
					}
					else{
						regression_lineaire(part->blocs[i][j], image_decime, posx, posy, 7);	
					}

					posy += 1;
				}
				posx += 1;
				posy = 0;
			}
			free_image(image_decime);
			free_image(image_extraite);
			free_image(image_iso);
		}
	}
    pthread_exit(NULL);
}


//fonction qui s'execute apres la phase initiale de compression,
//et qui recherche des bloc autour de celui selectionné avec un pas plus précis (de 1 pixel)
//Entrée : la partition part, l'image img et un entier pas qui donne la distance autour de laquel on cherche
void recherche_adjacente(partition* part, image* img, int pas){
	printf("Recherche adjacente\n");

    // Création des threads
    pthread_t threads[NUM_THREADS];
    threadArgs thread_args[NUM_THREADS];
    double step = (double)part->nbBlocHauteur / NUM_THREADS;

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_args[i].part = part;
        thread_args[i].img = img;
        thread_args[i].pasRecherche = pas;
        thread_args[i].start = (i == 0) ? 0 : thread_args[i-1].end;
        thread_args[i].end = (i == NUM_THREADS - 1) ? part->nbBlocHauteur : (int)((double)(i + 1) * step);
        pthread_create(&threads[i], NULL, recherche_adjacente_thread, (void *)&thread_args[i]);
    }

    // Attente de la fin de tous les threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
	printf("fin de recherche adjacente\n");
}


// Une fonction de debbug uniquement.
void affiche_partition(partition* part){
	for(int i = 0; i < part->nbBlocHauteur; i++){
		for(int j = 0; j < part->nbBlocLongueur; j++){
				printf("%d %d | ", (int)(part->blocs[i][j]->alpha * 10), (int)part->blocs[i][j]->beta);
		}
		printf("\n");
	}
}


//une itération de la décompression
void une_iteration(partition* part, image* source, image* resultat){
	double val_pixel;
	int h_bloc;
	int l_bloc;
	int n = source->niveau; //niveau

	for(int i = 0; i < part->nbBlocHauteur; i++){
		for(int j = 0; j < part->nbBlocLongueur; j++){
			h_bloc = part->blocs[i][j]->img->hauteur;
			l_bloc = part->blocs[i][j]->img->longueur;

			if(l_bloc == h_bloc){ //pas un bord
				//on prend en compte les isometrie
				int posx = part->blocs[i][j]->posx;
				int posy = part->blocs[i][j]->posy;

				image* image_extraite = create_image(h_bloc * 2, l_bloc * 2, n);
				image* image_decime = create_image(h_bloc, l_bloc, n);
				image* image_iso = create_image(h_bloc, l_bloc, n);

				extraire_image(posx, posy, posx + 2 * h_bloc - 1, posy + 2 * l_bloc - 1, source, image_extraite);
				decime_image(image_extraite, image_decime);
				isometrie(image_decime, image_iso, part->blocs[i][j]->id_iso);

				for(int k = 0; k < h_bloc; k++){
					for(int l = 0; l < l_bloc; l++){
						resultat->matrice[i * part->pas + k][j * part->pas + l] = image_iso->matrice[k][l] * part->blocs[i][j]->alpha - part->blocs[i][j]->beta;
					}
				}
				
				free_image(image_decime);
				free_image(image_extraite);
				free_image(image_iso);
			}
			else{
				int posx = part->blocs[i][j]->posx;
				int posy = part->blocs[i][j]->posy;
				for(int k = 0; k < h_bloc; k++){
					for(int l = 0; l < l_bloc; l++){
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

	remplissage(0, img1);
	remplissage(0, img2);

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

//prend une image et une autre image plus petite
//sort une image dont chaque pixel est le cout entre la petite image et celle extraite de la grande au pixel considéré, deux fois plus grande que la petite
image* analyse_un_bloc(image* img, image* echantillon, double sensibilite){
	assert(echantillon->niveau = img->niveau);
	int h = echantillon->hauteur;
	int l = echantillon->longueur;
	int n = echantillon->niveau;


	image* res = create_image(img->hauteur, img->longueur, img->niveau);

	image* image_extraite = create_image(h * 2, l * 2, n);
	image* image_decime = create_image(h, l, n);
	bloc* fixe = create_bloc(echantillon);

	int posx = 0;
	int posy = 0;

	double max = 1;
	
	while(posx < img->hauteur - 2 * h){
		while(posy < img->longueur - 2 * l){
			extraire_image(posx, posy, posx + h*2 - 1, posy + 2 * l - 1, img, image_extraite);
			decime_image(image_extraite, image_decime);

			regression_lineaire(fixe, image_decime, -1,-1,-1);
			if(fixe->cout > max) {
				max = fixe->cout;
			}
			res->matrice[posx][posy] = fixe->cout;
			//l'id iso est remis immédiatement à -1 donc le cout est actualisé par
			//l'appel dans tout les cas

			posy += 1;
			//printf("Test : %lf\n", fixe->cout);
		}
		posx += 1;
		posy = 0;
	}

	printf("Analyse : \nmax : %lf\n", max);
	multiply_number_image((double)n*sensibilite/max, res);
	
	free_image(image_decime);
	free_image(image_extraite);
	free(fixe);

	seuil_pixels(res);

	return res;
}

long double maxld(long double a, long double b){
		if(a > b)
		return a;
	else
		return b;
}

//calcule la constante de contraction en regardant sur des images tirées aléatoirement
long double test_contraction(partition* part, int nb_iteration){
	image* img1 = create_image(part->hauteur, part->longueur, part->niveau);
	image* img2 = create_image(part->hauteur, part->longueur, part->niveau);

	image* tmp1 = create_image(part->hauteur, part->longueur, part->niveau);
	image* tmp2 = create_image(part->hauteur, part->longueur, part->niveau);

	long double distance_antecedent;
	long double distance_image;

	long double res = 0;

	for(int i = 0;i < nb_iteration; i++){
		image_aleatoire(img1);
		image_aleatoire(img2);

		distance_antecedent = cout_images(img1, img2);
		printf("distance antecedent %Lf\n", distance_antecedent);
		
		une_iteration(part, img1, tmp1);
		une_iteration(part, img2, tmp2);

		distance_image = cout_images(tmp1, tmp2);
		printf("distance image %Lf\n", distance_image);

		res = maxld(distance_image / distance_antecedent, res);
	}

	free_image(img1);
	free_image(img2);
	free_image(tmp1);
	free_image(tmp2);

	return res;
}




int main()
{
	srand(time(NULL));
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

	// ------- Erreur entre deux images -------

	image* nuages = lecture_image("nuages.pgm");
	image* sans_iso = lecture_image("nuages_sans_iso_16_8_50.pgm");
	image* avec_iso = lecture_image("nuages_16_8_50.pgm");
	image* avec_am = lecture_image("nuages_16_8_50_am8.pgm");

	printf("sans iso %Lf\n", sqrtl(cout_images(nuages, sans_iso)));
	printf("avec iso %Lf\n", sqrtl(cout_images(nuages, avec_iso)));
	printf("avec am %Lf\n", sqrtl(cout_images(nuages, avec_am)));
	

	free_image(nuages);
	free_image(sans_iso);
	free_image(avec_iso);
	free_image(avec_am);

	///------ Analyse blocs ------
	/*//
	
	image* img4 = lecture_image("foret.pgm");
	image* img5 = create_image(8,8,img4->niveau);

	int posx = 498;
	int posy = 104;

	extraire_image(posx, posy, posx + img5->hauteur - 1, posy + img5->longueur - 1, img4, img5);
	save_image(img5, "echantillon_avant.pgm");

	image* img6 = analyse_un_bloc(img4, img5, 1);
	save_image(img6, "analyse.pgm");
	save_image(img5, "echantillon_apres.pgm");

	free_image(img6);
	free_image(img4);
	free_image(img5);
	
	*/// -------- Analyse compression ---------
	/*//

	partition* part = lecture_fractale("nuages_8_2_am1.ftl");

	printf("La constante de contraction est au moins de %Lf", test_contraction(part, 100));

	free_partition(part);


	
	*/// ------ Compression -------
	/*/

	//image* image_non_compresse = lecture_image("rayures.pgm");
	image* image_non_compresse = lecture_image("rayures.pgm");

	partition* partition_compresse = compression(image_non_compresse, 8, 8);
	//partition* partition_compresse = lecture_fractale("rayure_32_16_100_am1.ftl");

	recherche_adjacente(partition_compresse, image_non_compresse, 8);

	//partition* partition_compresse = lecture_fractale("nuages_16_8_50_am8.ftl");

	//save_fractale(partition_compresse, "avec_amelioration.ftl");

	//partition* partition_compresse = lecture_fractale("mangues_16_8_50_iso.ftl");
	
	image* resultat_decompression_2 = decompression(partition_compresse, 50);
	save_image(resultat_decompression_2, "rayures_8_8_50_am8.pgm");

	//affiche_partition(partition_compresse);

	free_partition(partition_compresse);
	free_image(resultat_decompression_2);
	free_image(image_non_compresse);

	*///


	return 0;
}


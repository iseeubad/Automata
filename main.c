#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>


typedef struct Transition {
    int etat_depart;
    int etat_arriver;
    char etiquetteTransition;
} Transition;

typedef struct {
    int etat_initiaux[10];
    int etat_finaux[10];
    char alphabet[20];
    int etats[20];
    int etats_inatteignables[10];
    int nb_etat_initiaux, nb_etat_finaux, nb_transitions, nb_lignes, nb_etat, nb_alphabet;
    Transition transitions[50];
} Automate;

Automate automate_minimise;

//les fonctions utilisés
bool est_mot_engendre(Automate *automate, char mot[]);
void tester_mots_fichier(Automate *automate, char nom_fichier[]);
void compter_lignes(FILE *fichier, Automate *automate);
void dernieres_lignes(FILE *fichier, Automate *automate);
void lire_fichier(Automate *automate, char nom_fichier[]);
void afficher_alphabet(Automate *automate);
void generer_fichier_dot(Automate *automate);
void afficher_image_png();
void afficher_etats(Automate *automate);
void afficher_etats_initiaux(Automate *automate);
void afficher_etats_finaux(Automate *automate);
void afficher_automate(Automate *automate);
Automate *union_automates(Automate *automate1,Automate *automate2);
Automate *produit_automates(Automate *automate1, Automate *automate2);
Automate *etoile_automate(Automate *automate);
Automate eliminerEpsilonTransitions(Automate automate);
Automate supprimerEpsilonTransitions(Automate automate);
void convert_to_dfa(Automate *automate);
void MooreMinimize(Automate *automate, Automate *automate_minimise);
bool est_etat_final(const Automate *automate, int etat);


// Fonction pour lire le fichier et initialiser la structure Automate
void lire_fichier(Automate *automate, char nom_fichier[]) {
    FILE *file;
    file = fopen(nom_fichier, "r");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }

    // Initialization of Automate
    for (int i = 0; i < 50; i++) {
        for (int j = 0; j < 50; j++) {
            automate->transitions[i].etat_depart = -1;
            automate->transitions[i].etat_arriver = -1;
            automate->transitions[i].etiquetteTransition = '\0';
        }
    }
    for (int i = 0; i < 10; i++) {
        automate->etat_initiaux[i] = -1;
        automate->etat_finaux[i] = -1;
    }
    for (int i = 0; i < 20; i++) {
        automate->alphabet[i] = '\0';
        automate->etats[i] = -1;
    }
    automate->nb_etat_initiaux = 0;
    automate->nb_etat_finaux = 0;
    automate->nb_transitions = 0;
    automate->nb_lignes = 0;
    automate->nb_etat = 0;
    automate->nb_alphabet = 0;

    compter_lignes(file, automate);

    int etatD, etata, nba = 0, nbe = 0, n = 0;
    char etiquette;
    bool alphabet_existe = false;

    while (n < automate->nb_lignes -1) {
        fscanf(file, "%d %d %c", &etatD, &etata, &etiquette);
        automate->transitions[n].etat_depart = etatD;
        automate->transitions[n].etat_arriver = etata;
        automate->transitions[n].etiquetteTransition = etiquette;
        automate->nb_transitions++;
        automate->etats[nbe++] = etata;
        automate->etats[nbe++] = etatD;
        for (int i = 0; i < 20; i++) {
            if (automate->alphabet[i] == etiquette) {
                alphabet_existe = true;
                break;
            }
        }
        if (!alphabet_existe) {
            automate->alphabet[nba++] = etiquette;
        }
        alphabet_existe = false;
        n++;
    }
    automate->nb_alphabet = nba;
    automate->nb_etat = nbe;
    dernieres_lignes(file, automate);

    fclose(file);
 // Trier et supprimer les doublons dans les états de l'automate
    int i, j, tmp;
    for (i = 0; i < automate->nb_etat - 1; i++)
    {
        for (j = 0; j < automate->nb_etat - i - 1; j++)
        {
            if (automate->etats[j] > automate->etats[j + 1])
            {
                tmp = automate->etats[j];
                automate->etats[j] =automate->etats[j + 1];
                automate->etats[j + 1] = tmp;
            }
        }
    }
    // (supprimer les doublons dans les états de l'automate)
    j = 0;
    for (i = 0; i <automate->nb_etat; i++)
        if (i == 0 || automate->etats[i] != automate->etats[i - 1])
            automate->etats[j++] = automate->etats[i];

    automate->nb_etat = j;
};
void afficher_alphabet(Automate *automate){
    printf("***************\n");
    printf("Les alphabets:\n");
    for (int i = 0; i < automate->nb_alphabet; i++) {
        printf("%c ",automate->alphabet[i]);
    }
    printf("\n");
    printf("***************\n");
}

// Function to generate the DOT file and display the PNG image
void generer_fichier_dot(Automate *automate) {
	int i,k=0;
    FILE *fichier = fopen("automate.dot", "w+");
    if (fichier == NULL) {
        perror("Erreur lors de la création du fichier DOT");
        exit(EXIT_FAILURE);
    }
    fprintf(fichier, "digraph G {\n");
    for (i = 0; i < automate->nb_transitions; i++) {
        int depart = automate->transitions[i].etat_depart;
        int arrivee = automate->transitions[i].etat_arriver;
        char etiquette = automate->transitions[i].etiquetteTransition;
        fprintf(fichier, "  %d -> %d [label=\"%c\"];\n", depart, arrivee, etiquette);
    }


    // etats inatteignables
    for (int i = 0; i <automate->nb_etat; i++) {
        int etat_inatteignable = 1;
        for (int j = 0; j < automate->nb_transitions; j++) {
            if (automate->transitions[j].etat_arriver == automate->etats[i] && automate->etats_inatteignables[j]!=automate->etats[i]  ) {
                etat_inatteignable = 0;
            }
        }
        if (etat_inatteignable) {
            automate->etats_inatteignables[k]=automate->etats[i];
            fprintf(fichier, "  %d ",automate->etats[i]);
            fprintf(fichier, "[shape = circle, style=filled, color = gray];\n");
            k++;
        }

    }
    // etats finaux
    for (i = 0; i < automate->nb_etat_finaux; i++) {
        fprintf(fichier, "  %d [shape=circle, style=filled, color=blue];\n", automate->etat_finaux[i]);
        }

    // etats initiaux
    for (i = 0; i < automate->nb_etat_initiaux; i++) {
    	fprintf(fichier, "  %d [shape=circle, style=filled, color=green];\n", automate->etat_initiaux[i]);
	}

    fprintf(fichier, "}\n");
    fclose(fichier);
}

//afficher image png
void afficher_image_png() {
    system("dot -Tpng automate.dot -o automate.png");
    system("start automate.png");
}

// Main function
int main() {
    Automate automate;
    Automate automate1;
    char nom_fichier[100];
    char nom_fichier1[100];
    int choix;
    char mot[100];
    int operation;

    do{
        printf("\nMenu Principal:\n");
        printf("1: Operation sur Une automate\n");
        printf("2: Operation sur 2 automate\n");
        printf("0: Exit\n\n");
        scanf("%d", &operation);

        if(operation==1){
            Automate automate_etoile;
            Automate automateSansEpsilon;
            printf("Donnez le nom du fichier .txt : ");
            scanf("%s", nom_fichier);
            lire_fichier(&automate, nom_fichier);


            do {
                printf("\nMenu une automate:\n");
                printf("1: Afficher les alphabets\n");
                printf("2: Generer le fichier dot et afficher l'image png\n");
                printf("3: Afficher Tous les etats\n");
                printf("4: Afficher les etats initiaux\n");
                printf("5: Afficher les etats finaux\n");
                printf("6: Tester si un mot est engendré\n");
                printf("7: Tester les mots d'un fichier\n");
                printf("8: Etoile de l'automate\n");
                printf("9: Supprimer les transitions epsilon\n");
                printf("10: Afficher l'automate deterministe\n");
                printf("11: Minimiser automate\n");
                printf("0: Exit\n\n");
                scanf("%d", &choix);

                switch (choix) {
                    case 1:
                        printf("Les alphabets de l'automate :\n");
                        afficher_alphabet(&automate);
                        break;
                    case 2:
                        generer_fichier_dot(&automate);
                        afficher_image_png();
                        break;
                    case 3:
                        printf("Tous les etats de l'automate :\n");
                        afficher_etats(&automate);
                        break;
                    case 4:
                        printf("Les etats initiaux de l'automate :\n");
                        afficher_etats_initiaux(&automate);
                        break;
                    case 5:
                        printf("Les etats finaux de l'automate :\n");
                        afficher_etats_finaux(&automate);
                        break;
                    case 6:
                        printf("Entrez le mot à tester : ");
                        scanf("%s", mot);
                        if (est_mot_engendre(&automate, mot)) {
                            printf("%s est engendré.\n", mot);
                        } else {
                            printf("%s n'est pas engendré.\n", mot);
                        }
                        break;
                    case 7:
                        printf("Entrez le nom du fichier à tester : ");
                        scanf("%s", nom_fichier);
                        tester_mots_fichier(&automate, nom_fichier);
                        break;
                    case 8:
                        automate_etoile = *etoile_automate(&automate);
                        printf("etoile effectuée avec succès !\n");
                        afficher_automate(&automate_etoile);
                        generer_fichier_dot(&automate_etoile);
                        afficher_image_png();
                        free(etoile_automate);
                        break;
                    case 9:
                        automateSansEpsilon = eliminerEpsilonTransitions(automate);
                        automateSansEpsilon = supprimerEpsilonTransitions(automateSansEpsilon);
                        printf("suppression des epsilon s'effectuée avec succès !\n");
                        afficher_automate(&automateSansEpsilon);
                        generer_fichier_dot(&automateSansEpsilon);
                        afficher_image_png();
                        break;
                    case 10:
                        convert_to_dfa(&automate);
                        break;
                    case 11:
                        memset(&automate_minimise, 0, sizeof(Automate));
                        MooreMinimize(&automate, &automate_minimise);
                        generer_fichier_dot(&automate_minimise);
                        afficher_image_png();
                        break;
                    case 0:
                        printf("retourne au menu principal !\n");
                        break;
                    default:
                        printf("Choix invalide. Veuillez saisir à nouveau.\n");
                }
            } while (choix != 0);
    }
    else if(operation==2){
        Automate automate_union;
        Automate automate_produit;
        printf("Donnez le nom du premier fichier .txt :\n");
        scanf("%s", nom_fichier);
        lire_fichier(&automate, nom_fichier);
        printf("Donnez le nom du deuxieme fichier .txt :\n");
        scanf("%s", nom_fichier1);
        lire_fichier(&automate1, nom_fichier1);

        do{
            printf("\nMenu deux automates:\n");
            printf("1: Union des automates\n");
            printf("2: Produit des automates\n");
            printf("0: Exit\n\n");
            scanf("%d", &choix);

            switch (choix) {
                case 1:
                    automate_union = *union_automates(&automate, &automate1);
                    printf("Union effectuée avec succès !\n");
                    afficher_automate(&automate_union);
                    generer_fichier_dot(&automate_union);
                    afficher_image_png();
                    //free(union_automates);
                    break;
                case 2:
                    automate_produit = *produit_automates(&automate, &automate1);
                    printf("Produit effectuée avec succès !\n");
                    generer_fichier_dot(&automate);
                    afficher_image_png();
                    generer_fichier_dot(&automate1);
                    afficher_image_png();
                    //free(produit_automates);
                    break;
                case 0:
                    printf("retourne au menu principal !\n");
                    break;
                default:
                    printf("Choix invalide. Veuillez saisir à nouveau.\n");
            }
        }while(choix!=0);
    }
    else {
        printf("Au revoir !\n");
        break;

    }
    }while (operation != 0);

    return 0;
}

// Function to count lines in the file
void compter_lignes(FILE *fichier, Automate *automate) {
    int ch;
    automate->nb_lignes = 0;
    while ((ch = fgetc(fichier)) != EOF)
        if (ch == '\n')
            automate->nb_lignes++;
    rewind(fichier);
}
//Dernier lignes
void dernieres_lignes(FILE *fichier, Automate *automate) {
    int n, ligne = automate->nb_lignes - 1;
    int i = 0;

    while (ligne++ < automate->nb_lignes) {
        fscanf(fichier, "%d", &n);
        automate->etat_initiaux[i++] = n;
        automate->nb_etat_initiaux++;
    }
    fgetc(fichier);
    i = 0;

    while (fscanf(fichier, "%d", &n) == 1) {
        automate->etat_finaux[i++] = n;
        automate->nb_etat_finaux++;
    }
}

//affichage des etats
void afficher_etats(Automate *automate) {
    printf("***************\n");
    printf("Tous les états:\n");
    for (int i = 0; i < automate->nb_etat; i++) {
        printf("%d ", automate->etats[i]);
    }
    printf("\n");
    printf("***************\n");
}

void afficher_etats_initiaux(Automate *automate) {
    printf("***************\n");
    printf("Les états initiaux:\n");
    for (int i = 0; i < automate->nb_etat_initiaux; i++) {
        printf("%d ", automate->etat_initiaux[i]);
    }
    printf("\n");
    printf("***************\n");
}

void afficher_etats_finaux(Automate *automate) {
    printf("***************\n");
    printf("Les états finaux:\n");
    for (int i = 0; i < automate->nb_etat_finaux; i++) {
        printf("%d ", automate->etat_finaux[i]);
    }
    printf("\n");
    printf("***************\n");
}

// Function to test if a word is generated by the automaton
bool est_mot_engendre(Automate *automate, char mot[]) {
    int etat_actuel = automate->etat_initiaux[0];
    int i = 0;

    while (mot[i] != '\0') {
        char symbole = mot[i++];
        int etat_suivant = -1;

        for (int j = 0; j < automate->nb_transitions; j++) {
            if (automate->transitions[j].etat_depart == etat_actuel && automate->transitions[j].etiquetteTransition == symbole) {
                etat_suivant = automate->transitions[j].etat_arriver;
                break;
            }
        }

        if (etat_suivant == -1)
            return false;

        etat_actuel = etat_suivant;
    }

    for (int j = 0; j < automate->nb_etat_finaux; j++) {
        if (etat_actuel == automate->etat_finaux[j])
            return true;
    }

    return false;
}

// Function to test words from the .txt file
void tester_mots_fichier(Automate *automate, char nom_fichier[]) {
    FILE *file;
    file = fopen(nom_fichier, "r");
    if (file == NULL) {
        perror("Erreur lors de l'ouverture du fichier");
        exit(EXIT_FAILURE);
    }

    char mot[100];
    while (fscanf(file, "%s", mot) != EOF) {
        printf("********************");
        if (est_mot_engendre(automate, mot)) {
            printf("\n");
            printf("%s : Accepté\n", mot);
        } else {
            printf("\n");
            printf("%s : Non accepté\n", mot);
        }
        printf("********************");
    }

    fclose(file);
}
void afficher_automate(Automate *automate) {
    printf("Etats initiaux :\n");
    for (int i = 0; i < automate->nb_etat_initiaux; i++) {
        printf("%d ", automate->etat_initiaux[i]);
    }
    printf("\n");

    printf("Etats finaux :\n");
    for (int i = 0; i < automate->nb_etat_finaux; i++) {
        printf("%d ", automate->etat_finaux[i]);
    }
    printf("\n");

    printf("Transitions :\n");
    for (int i = 0; i < automate->nb_transitions; i++) {
        printf("De l'état %d à l'état %d avec l'étiquette '%c'\n",
               automate->transitions[i].etat_depart,
               automate->transitions[i].etat_arriver,
               automate->transitions[i].etiquetteTransition);
    }
}


//union de deux automates
Automate *union_automates(Automate *automate1, Automate *automate2) {
    Automate *automate_union = malloc(sizeof(Automate)); // Allocation dynamique de la mémoire pour l'automate d'union
    if (automate_union == NULL) {
        perror("Erreur d'allocation mémoire");
        exit(EXIT_FAILURE);
    }

    // Initialisation de l'automate d'union
    memset(automate_union, 0, sizeof(Automate));

     // Union des alphabets
    char *alphabet_union = strcat(automate1->alphabet, automate2->alphabet);
    strcpy(automate_union->alphabet, alphabet_union);

    // Union des états initiaux
    memcpy(automate_union->etat_initiaux, automate1->etat_initiaux, automate1->nb_etat_initiaux * sizeof(int));
    automate_union->nb_etat_initiaux = automate1->nb_etat_initiaux;
    for (int i = 0; i < automate2->nb_etat_initiaux; i++) {
        int etat = automate2->etat_initiaux[i];
        bool existe = false;
        for (int j = 0; j < automate1->nb_etat_initiaux; j++) {
            if (etat == automate1->etat_initiaux[j]) {
                existe = true;
                break;
            }
        }
        if (!existe) {
            automate_union->etat_initiaux[automate_union->nb_etat_initiaux++] = etat;
        }
    }

    // Union des états finaux
    memcpy(automate_union->etat_finaux, automate1->etat_finaux, automate1->nb_etat_finaux * sizeof(int));
    automate_union->nb_etat_finaux = automate1->nb_etat_finaux;
    for (int i = 0; i < automate2->nb_etat_finaux; i++) {
        int etat = automate2->etat_finaux[i];
        bool existe = false;
        for (int j = 0; j < automate1->nb_etat_finaux; j++) {
            if (etat == automate1->etat_finaux[j]) {
                existe = true;
                break;
            }
        }
        if (!existe) {
            automate_union->etat_finaux[automate_union->nb_etat_finaux++] = etat;
        }
    }

    // Union des transitions
    memcpy(automate_union->transitions, automate1->transitions, automate1->nb_transitions * sizeof(Transition));
    automate_union->nb_transitions = automate1->nb_transitions;
    for (int i = 0; i < automate2->nb_transitions; i++) {
        Transition transition = automate2->transitions[i];
        bool existe = false;
        for (int j = 0; j < automate1->nb_transitions; j++) {
            if (transition.etat_depart == automate1->transitions[j].etat_depart &&
                transition.etat_arriver == automate1->transitions[j].etat_arriver &&
                transition.etiquetteTransition == automate1->transitions[j].etiquetteTransition) {
                existe = true;
                break;
            }
        }
        if (!existe) {
            automate_union->transitions[automate_union->nb_transitions++] = transition;
        }
    }
    // Retourner le pointeur vers l'automate d'union
    return automate_union;
}

//produit de deux automates
Automate *produit_automates(Automate *automate1, Automate *automate2) {
    // Allocation dynamique de la mémoire pour l'automate produit
    Automate *produit_automate = malloc(sizeof(Automate));
    if (produit_automate == NULL) {
        perror("Erreur d'allocation mémoire");
        exit(EXIT_FAILURE);
    }

    // Initialisation de l'automate produit
    memset(produit_automate, 0, sizeof(Automate));

    printf("Produit des deux Automates:\n");

    // Afficher l'alphabet commun
    printf("Alphabet: { ");
    for (int i = 0; i < automate1->nb_alphabet; i++) {
        if (strchr(automate2->alphabet, automate1->alphabet[i]) != NULL) {
            printf("%c ", automate1->alphabet[i]);
        }
    }
    printf("}\n");

    // Calculer et afficher le produit des états initiaux
    printf("Etats initiaux du produit: { ");
    for (int i = 0; i < automate1->nb_etat_initiaux; i++) {
        for (int j = 0; j < automate2->nb_etat_initiaux; j++) {
            printf("(%d,%d) ", automate1->etat_initiaux[i], automate2->etat_initiaux[j]);
        }
    }
    printf("}\n");

    // Calculer et afficher le produit des états finaux
    printf("Etats finaux du produit: { ");
    for (int i = 0; i < automate1->nb_etat_finaux; i++) {
        for (int j = 0; j < automate2->nb_etat_finaux; j++) {
            printf("(%d,%d) ", automate1->etat_finaux[i], automate2->etat_finaux[j]);
        }
    }
    printf("}\n");

    // Calculer et afficher le produit des transitions
    printf("Transitions du produit:\n");
    for (int i = 0; i < automate1->nb_transitions; i++) {
        for (int j = 0; j < automate2->nb_transitions; j++) {
            if (automate1->transitions[i].etiquetteTransition == automate2->transitions[j].etiquetteTransition) {
                printf("((%d,%d), %c, (%d,%d))\n",
                    automate1->transitions[i].etat_depart,
                    automate2->transitions[j].etat_depart,
                    automate1->transitions[i].etiquetteTransition,
                    automate1->transitions[i].etat_arriver,
                    automate2->transitions[j].etat_arriver);
            }
        }
    }
    // Retourner le pointeur vers l'automate produit
    return produit_automate;

}
//etoile d'automate
Automate *etoile_automate(Automate *automate) {
    // Allocation dynamique de la mémoire pour l'automate étoile
    Automate *automate_etoile = malloc(sizeof(Automate));
    if (automate_etoile == NULL) {
        perror("Erreur d'allocation mémoire");
        exit(EXIT_FAILURE);
    }

    // Copie de l'automate original dans l'automate étoile
    memcpy(automate_etoile, automate, sizeof(Automate));
        // Trouver la valeur la plus élevée parmi tous les états
    int max_etat = -1;
    for (int i = 0; i < automate_etoile->nb_etat; i++) {
        if (automate_etoile->etats[i] > max_etat) {
            max_etat = automate_etoile->etats[i];
        }
    }
    int nouvel_etat_initial = max_etat + 1;

    // Ajout d'un nouvel état initial qui est aussi un état final
    automate_etoile->etat_initiaux[automate_etoile->nb_etat_initiaux++] = nouvel_etat_initial;
    automate_etoile->etat_finaux[automate_etoile->nb_etat_finaux++] = nouvel_etat_initial;
    automate_etoile->etats[automate_etoile->nb_etat++] = nouvel_etat_initial;

    // Ajout de transitions epsilon de chaque état final vers le nouvel état initial
    for (int i = 0; i < automate_etoile->nb_etat_finaux - 1; i++) { // -1 pour exclure le nouvel état final
        Transition nouvelle_transition = {automate_etoile->etat_finaux[i], nouvel_etat_initial, '3'};
        automate_etoile->transitions[automate_etoile->nb_transitions++] = nouvelle_transition;
    }

    // Ajout de transitions epsilon du nouvel état initial vers les anciens états initiaux
    for (int i = 0; i < automate_etoile->nb_etat_initiaux - 1; i++) { // -1 pour exclure le nouvel état initial
        Transition nouvelle_transition = {nouvel_etat_initial, automate_etoile->etat_initiaux[i], '3'};
        automate_etoile->transitions[automate_etoile->nb_transitions++] = nouvelle_transition;
    }

    // Retourner le pointeur vers l'automate étoile
    return automate_etoile;
}

//fonction pour supprimer les transitions epsilon
Automate eliminerEpsilonTransitions(Automate automate) {
    Automate sansEpsilon;
    memcpy(&sansEpsilon, &automate, sizeof(Automate)); // Copie de l'automate
    sansEpsilon.nb_transitions = 0; // Réinitialisation du nombre de transitions

    // Identifier les transitions epsilon et construire la fermeture transitive
    int fermeture[20][20] = {0};
    for (int i = 0; i < automate.nb_transitions; i++) {
        if (automate.transitions[i].etiquetteTransition == '3') {
            fermeture[automate.transitions[i].etat_depart][automate.transitions[i].etat_arriver] = 1;
        }
    }

    // Calcul de la fermeture transitive
    for (int k = 0; k < automate.nb_etat; k++) {
        for (int i = 0; i < automate.nb_etat; i++) {
            for (int j = 0; j < automate.nb_etat; j++) {
                fermeture[i][j] |= (fermeture[i][k] && fermeture[k][j]);
            }
        }
    }

    // Éliminer les transitions epsilon et ajouter des transitions directes
    for (int i = 0; i < automate.nb_etat; i++) {
        for (int j = 0; j < automate.nb_etat; j++) {
            if (fermeture[i][j]) {
                for (int k = 0; k < automate.nb_alphabet; k++) {
                    char symbole = automate.alphabet[k];
                    for (int l = 0; l < automate.nb_transitions; l++) {
                        if (automate.transitions[l].etat_depart == j && automate.transitions[l].etiquetteTransition == symbole) {
                            Transition nouvelleTransition = {i, automate.transitions[l].etat_arriver, symbole};
                            sansEpsilon.transitions[sansEpsilon.nb_transitions++] = nouvelleTransition;
                        }
                    }
                }
            }
        }
    }

    return sansEpsilon;
}


Automate supprimerEpsilonTransitions(Automate automate) {
    Automate sansEpsilon;
    memcpy(&sansEpsilon, &automate, sizeof(Automate)); // Copie de l'automate
    sansEpsilon.nb_transitions = 0; // Réinitialisation du nombre de transitions

    bool atteignable[20] = {false};
    for (int i = 0; i < sansEpsilon.nb_etat_initiaux; i++) {
        atteignable[sansEpsilon.etat_initiaux[i]] = true; // Les états initiaux sont atteignables
    }

    // Élimination des transitions epsilon
    for (int i = 0; i < automate.nb_transitions; i++) {
        if (automate.transitions[i].etiquetteTransition != '3') {
            sansEpsilon.transitions[sansEpsilon.nb_transitions++] = automate.transitions[i];
            atteignable[automate.transitions[i].etat_arriver] = true;
        }
    }

    // Suppression des états inatteignables
    int nb_transitions_sans_inatteignables = 0;
    for (int i = 0; i < sansEpsilon.nb_transitions; i++) {
        if (atteignable[sansEpsilon.transitions[i].etat_depart]) {
            sansEpsilon.transitions[nb_transitions_sans_inatteignables++] = sansEpsilon.transitions[i];
        }
    }
    sansEpsilon.nb_transitions = nb_transitions_sans_inatteignables;

    return sansEpsilon;
}


/////determinisation:
void convert_to_dfa(Automate *automate) {
    int nb_etats_dfa = 1 << (automate->nb_etat);  // Maximum number of states in the DFA
    Automate dfa;
    memset(&dfa, 0, sizeof(Automate));

    // Initialize the DFA alphabet
    memcpy(dfa.alphabet, automate->alphabet, sizeof(automate->alphabet));
    dfa.nb_alphabet = automate->nb_alphabet;

    // Initialize the set of states in the DFA
    int etat_initial_dfa = 0;
    for (int i = 0; i < automate->nb_etat_initiaux; i++) {
        etat_initial_dfa |= (1 << automate->etat_initiaux[i]);
    }
    dfa.etat_initiaux[0] = etat_initial_dfa;
    dfa.nb_etat_initiaux = 1;

    // Initialize the set of final states in the DFA
    for (int etat = 0; etat < nb_etats_dfa; etat++) {
        int est_final = 0;
        for (int i = 0; i < automate->nb_etat; i++) {
            if ((etat & (1 << i)) && est_etat_final(automate, i)) {
                est_final = 1;
                break;
            }
        }
        if (est_final) {
            dfa.etat_finaux[dfa.nb_etat_finaux++] = etat;
        }
    }

    // Compute the DFA transition function
    int nb_etats_dfa_atteints = 1;
    dfa.etats[0] = etat_initial_dfa;
    for (int etat_courant = 0; etat_courant < nb_etats_dfa_atteints; etat_courant++) {
        int etat_dfa_courant = dfa.etats[etat_courant];
        for (int symbole = 0; symbole < dfa.nb_alphabet; symbole++) {
            int etat_dfa_suivant = 0;
            for (int i = 0; i < automate->nb_etat; i++) {
                if (etat_dfa_courant & (1 << i)) {
                    int etat_suivant = transition(automate, i, symbole);
                    if (etat_suivant != -1) {
                        etat_dfa_suivant |= (1 << etat_suivant);
                    }
                }
            }
            int indice_etat_dfa_suivant = -1;
            for (int i = 0; i < nb_etats_dfa_atteints; i++) {
                if (dfa.etats[i] == etat_dfa_suivant) {
                    indice_etat_dfa_suivant = i;
                    break;
                }
            }
            if (indice_etat_dfa_suivant == -1) {
                dfa.etats[nb_etats_dfa_atteints] = etat_dfa_suivant;
                indice_etat_dfa_suivant = nb_etats_dfa_atteints++;
            }
            dfa.transitions[etat_courant * dfa.nb_alphabet + symbole].etat_depart = etat_dfa_courant;
            dfa.transitions[etat_courant * dfa.nb_alphabet + symbole].etat_arriver = dfa.etats[indice_etat_dfa_suivant];
            dfa.transitions[etat_courant * dfa.nb_alphabet + symbole].etiquetteTransition = dfa.alphabet[symbole];
            dfa.nb_transitions++;
        }
    }

    dfa.nb_etat = nb_etats_dfa_atteints;

    // Copy the DFA to the original automaton
    memcpy(automate, &dfa, sizeof(Automate));

    // Generate the DOT file and display the PNG image
    generer_fichier_dot(automate);
    afficher_image_png();
}


#define MAX_ETATS 100

void MooreMinimize(Automate *automate, Automate *automate_minimise) {
    int i, j, k;
    int nb_etats_minimises;
    bool table[MAX_ETATS][MAX_ETATS];

    // Initialiser la table de marquage
    for (i = 0; i < automate->nb_etat; i++) {
        for (j = 0; j < automate->nb_etat; j++) {
            table[i][j] = est_etat_final(automate, i) != est_etat_final(automate, j);
        }
    }

    // Itérer jusqu'à ce que la table de marquage soit stable
    bool changement;
    do {
        changement = false;
        for (i = 0; i < automate->nb_etat; i++) {
            for (j = 0; j < automate->nb_etat; j++) {
                if (!table[i][j]) {
                    for (k = 0; k < automate->nb_alphabet; k++) {
                        int etat_i = transition(automate, i, k);
                        int etat_j = transition(automate, j, k);
                        if (etat_i != -1 && etat_j != -1 && table[etat_i][etat_j]) {
                            table[i][j] = true;
                            changement = true;
                            break;
                        }
                    }
                }
            }
        }
    } while (changement);

    // Compter le nombre d'états minimisés
    nb_etats_minimises = 0;
    for (i = 0; i < automate->nb_etat; i++) {
        bool deja_compte = false;
        for (j = 0; j < i; j++) {
            if (!table[i][j] && !table[j][i]) {
                deja_compte = true;
                break;
            }
        }
        if (!deja_compte) {
            nb_etats_minimises++;
        }
    }

    // Construire l'automate minimisé
    automate_minimise->nb_etat = nb_etats_minimises;
    automate_minimise->nb_etat_finaux = 0;
    automate_minimise->nb_etat_initiaux = 0;
    automate_minimise->nb_transitions = 0;
    automate_minimise->nb_alphabet = automate->nb_alphabet;
    memcpy(automate_minimise->alphabet, automate->alphabet, sizeof(automate->alphabet));

    int *nouvel_etat = calloc(automate->nb_etat, sizeof(int));
    int etat_courant = 0;

    for (i = 0; i < automate->nb_etat; i++) {
        bool deja_compte = false;
        for (j = 0; j < i; j++) {
            if (!table[i][j] && !table[j][i]) {
                deja_compte = true;
                nouvel_etat[i] = nouvel_etat[j];
                break;
            }
        }
        if (!deja_compte) {
            nouvel_etat[i] = etat_courant++;
            if (est_etat_final(automate, i)) {
                automate_minimise->etat_finaux[automate_minimise->nb_etat_finaux++] = nouvel_etat[i];
            }
        }
    }

    for (i = 0; i < automate->nb_etat_initiaux; i++) {
        automate_minimise->etat_initiaux[automate_minimise->nb_etat_initiaux++] = nouvel_etat[automate->etat_initiaux[i]];
    }

    for (i = 0; i < automate->nb_etat; i++) {
        for (j = 0; j < automate->nb_alphabet; j++) {
            int etat_depart = i;
            int etat_arriver = transition(automate, etat_depart, j);
            if (etat_arriver != -1) {
                automate_minimise->transitions[automate_minimise->nb_transitions].etat_depart = nouvel_etat[etat_depart];
                automate_minimise->transitions[automate_minimise->nb_transitions].etat_arriver = nouvel_etat[etat_arriver];
                automate_minimise->transitions[automate_minimise->nb_transitions].etiquetteTransition = automate->alphabet[j];
                automate_minimise->nb_transitions++;
            }
        }
    }
    remove_duplicate_transitions(automate_minimise);
    free(nouvel_etat);
}

bool est_etat_final(const Automate *automate, int etat) {
    for (int i = 0; i < automate->nb_etat_finaux; i++) {
        if (automate->etat_finaux[i] == etat) {
            return true;
        }
    }
    return false;
}

int transition(const Automate *automate, int etat, int symbole) {
    for (int i = 0; i < automate->nb_transitions; i++) {
        if (automate->transitions[i].etat_depart == etat && automate->transitions[i].etiquetteTransition == automate->alphabet[symbole]) {
            return automate->transitions[i].etat_arriver;
        }
    }
    return -1; // Aucune transition trouvée
}

void remove_duplicate_transitions(Automate *automate) {
    int i, j;
    Transition *temp_transitions = malloc(automate->nb_transitions * sizeof(Transition));
    int temp_nb_transitions = 0;

    // Copy unique transitions to temp_transitions
    for (i = 0; i < automate->nb_transitions; i++) {
        bool duplicate = false;
        for (j = 0; j < temp_nb_transitions; j++) {
            if (temp_transitions[j].etat_depart == automate->transitions[i].etat_depart &&
                temp_transitions[j].etat_arriver == automate->transitions[i].etat_arriver &&
                temp_transitions[j].etiquetteTransition == automate->transitions[i].etiquetteTransition) {
                duplicate = true;
                break;
            }
        }
        if (!duplicate) {
            temp_transitions[temp_nb_transitions++] = automate->transitions[i];
        }
    }

    // Copy temp_transitions back to automate->transitions
    memcpy(automate->transitions, temp_transitions, temp_nb_transitions * sizeof(Transition));
    automate->nb_transitions = temp_nb_transitions;

    free(temp_transitions);
}

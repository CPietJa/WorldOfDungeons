/**
 * \file test_liste.c
 * \brief Programme qui teste que le module liste.
 * \author GALBRUN Tibane
 * \date 07/03/2019
 * \version 0.1
*/

#include "test_liste.h"
#include <liste.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

int main() {
  printf("----- Programme de Test de Liste Générique -----\n");

  /* Valeurs */
  /*float * a = malloc(sizeof(float)); *a = 1.1;
    float * b = malloc(sizeof(float)); *b = 2.2;
    float * c = malloc(sizeof(float)); *c = 3.3;*/
  float a = 3, b = 4, c = 5;

  /* Déclaration des listes */
  t_liste j1;

  /* Création des listes */
  printf("\t--> Création de la Liste Générique\n");
  init_liste(&j1);
  printf("\t\t-- OK\n");

  /* Remplissage des listes avec des valeurs quelconques */
  printf("\t--> Remplissage de la Liste Générique\n");
  en_tete(&j1);
  assert(ajout_droit(&j1, &a) == OK);
  assert(ajout_droit(&j1, &b) == OK);
  assert(ajout_droit(&j1, &c) == OK);
  printf("\t\t-- OK\n");

  /* Affichage des listes */
  printf("\t--> Affichage de la Liste Générique\n");
  afficher_liste(&j1);
  printf("\t\t-- OK\n");

  /* Récupération d'une valeur dans la liste */
  float * val;
  printf("\t--> Récupération de la valeur 2 de la liste\n");
  assert(valeur_liste(&j1,1,(void **)&val) == OK);
  printf("Valeur : %.2f\n", *val);
  printf("\t\t-- OK\n");

  /* Recherche de valeur dans la liste */
  float val2 = 5;
  printf("\t--> Récupération de la valeur 5\n");
  printf("Valeur trouvée ? : %d\n", recherche_liste(&j1,(void *)&val2, comparer_float));
  printf("\t\t-- OK\n");

  /* Destruction des listes */
  printf("\t--> Destruction des Listes\n");
  detruire_liste(&j1, NULL);
  printf("\t\t-- OK\n");

  return EXIT_SUCCESS;
}

void afficher_liste(t_liste *p) {
  for (en_tete(p); !hors_liste(p); suivant(p)) {
    float *val;
    valeur_elt(p, (void **)&val);
    printf("%.2f ", *val);
  }
  printf("\n");
}

int comparer_float(void * f1, void * f2)
{
  return *(float*)f1 == *(float*)f2;
}
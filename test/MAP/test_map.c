#include <affichage.h>
#include <block.h>
#include <couleurs.h>
#include <erreur.h>
#include <generation.h>
#include <liste.h>
#include <map.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char const *argv[]) {
  t_map *map = NULL;
  int *tab;
  int boucle = 0;
  block_type2_t blocks2[NB_BLOCK] = {{AIR, "air", NOIR},      {HERBE, "herbe", VERT},     {TERRE, "terre", MARRON}, {EAU, "eau", BLEU},
                                     {SABLE, "sable", JAUNE}, {FEUILLE, "feuille", VERT}, {BOIS, "bois", MARRON},   {ROCHE, "roche", GRIS},
                                     {NEIGE, "neige", BLANC}, {GLACE, "glace", CYAN}};

  printf("Création de la map et du repertoire : --");
  aff_erreur(MAP_creer(&map, "World", 121212));
  printf("\n");

  printf("Nom map : %s avec comme SEED : %d\n", map->nom, map->SEED);

  while (boucle++ <= 50) {
    gen_col(&tab, boucle);
    en_queue(map->list);
    ajout_droit(map->list, tab);
  }

  aff_map(map->list, 30, 80, blocks2);

  printf("Destruction de la zone mémoire pointer par map : -- ");
  aff_erreur(MAP_detruction(&map));
  printf("\n");

  printf("Chargement d'une map : -- ");
  aff_erreur(MAP_charger(&map, "World"));
  printf("\n");

  printf("Nom map : %s avec comme SEED : %d\n", map->nom, map->SEED);

  printf("Suppression d'une map : -- ");
  aff_erreur(MAP_supprimer(map));
  printf("\n");

  printf("Destruction de la zone mémoire pointer par map : -- ");
  aff_erreur(MAP_detruction(&map));
  printf("\n");
  return 0;
}

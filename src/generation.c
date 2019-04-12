/**
*   \file generation.c
*   \brief Fonction de génération procédural grace a Perlin Noise
*   \author {Maxence.D}
*   \version 0.1
*   \date 10 mars 2019
*
*   \fn void init_map(t_block *map)
*   \brief Initialise un tableau de block
*
*   \fn int gen_col(t_block **tab, int x)
*   \brief Génere une colone de block
*
**/

#include <biome.h>
#include <block.h>
#include <commun.h>
#include <generation.h>
#include <perlin.h>
#include <stdio.h>
#include <stdlib.h>
#include <structure_block.h>
#include <touches.h>

void init_map(t_block *map, int x) {
  int i;
  for (i = 0; i < MAX; i++) {
    map[i].id = AIR;
    map[i].x = x;
    map[i].y = i;
    map[i].plan = ARRIERE_PLAN;
  }
}

void gen_minerai(t_block *tab, int x, int taille_max) {
  double rnd;
  for (int j = 0; j <= taille_max - HAUTEUR_SURFACE; j++) {
    rnd = perlin2d(x, j, 0.1, DEPTH);

    if ((rnd >= 0.73555 && rnd <= 0.74) && j >= 30 && j <= HAUTEUR_MINIMUN) {
      tab[j].id = MINERAI_CHARBON;
      tab[j].y = j;
      tab[j].x = x;
      tab[j].plan = PREMIER_PLAN;
      printf("MINERAI_CHARBON %f\n", rnd);
    } else if ((rnd >= 0.43555 && rnd <= 0.44) && j <= HAUTEUR_MINIMUN && j >= 30) {
      tab[j].id = MINERAI_CUIVRE;
      tab[j].y = j;
      tab[j].x = x;
      tab[j].plan = PREMIER_PLAN;
      printf("MINERAI_CUIVRE %f\n", rnd);
    } else if ((rnd >= 0.39111 && rnd <= 0.39333) && j <= 30) {
      tab[j].id = MINERAI_OR;
      tab[j].y = j;
      tab[j].x = x;
      tab[j].plan = PREMIER_PLAN;
      printf("MINERAI_OR %f\n", rnd);
    }
  }
}

int gen_col(t_liste *list, int x, int dir) {
  int j;
  double rnd;
  int taille_max = (perlin2d(x, MAX / 2, FREQ, DEPTH) * MAX / 2) + HAUTEUR_MINIMUN;

  t_block *tab = malloc(sizeof(t_block) * MAX);

  init_map(tab, x);

  biome = BIOME_change_type(x, taille_max);

  STRUCT_generation(x, taille_max, dir, tab);

  t_biome *b = BIOME_rechercheParType(biome);

  /* génération eau */
  // for (j = 0; taille_max < HAUTEUR_EAU; j++) {
  //   if (j + taille_max == HAUTEUR_EAU - 1 && (biome == TOUNDRA || biome == TAIGA)) {
  //     (*tab)[taille_max + j].id = GLACE;
  //     (*tab)[taille_max + j].y = taille_max + j;
  //     (*tab)[taille_max + j].x = x;
  //   } else if ((*tab)[taille_max + j].id == AIR) {
  //     (*tab)[taille_max + j].id = EAU;
  //     (*tab)[taille_max + j].y = taille_max + j;
  //     (*tab)[taille_max + j].x = x;
  //   }
  // }

  if (b)
    for (int i = 0, layer = 0; i < b->nb_layers; i++) {
      for (int j = layer, k = 0; k < b->layers[i].nb_couche && layer <= taille_max; j++, layer++, k++) {
        tab[taille_max - j].id = b->layers[i].block_type;
        tab[taille_max - j].y = taille_max - j;
        tab[taille_max - j].x = x;
        tab[taille_max - j].plan = PREMIER_PLAN;
      }
    }

  gen_minerai(tab, x, taille_max);
  /* Génération profondeur */

  for (j = 0; j <= taille_max - HAUTEUR_SURFACE; j++) {
    rnd = perlin2d(x, j, FREQ, DEPTH) * MAX;
    if (taille_max - j > PROFONDEUR_GROTTE && j >= BEDROCK && rnd >= (MAX / 2 - SIZE_GROTTE) && rnd <= (MAX / 2 + SIZE_GROTTE)) { /* grotte */
      tab[j].id = AIR;
      tab[j].y = j;
      tab[j].x = x;
      tab[j].plan = ARRIERE_PLAN;
    }
  }

  if (dir == DROITE) {
    en_queue(list);
    ajout_droit(list, tab);
    // printf("Gen i : %d DROITE\n", x);
  }

  if (dir == GAUCHE) {
    en_tete(list);
    ajout_gauche(list, tab);
    // printf("Gen i : %d GAUCHE\n", x);
  }

  return taille_max;
}

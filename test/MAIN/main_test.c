/**
 *
 *   \file main_test.c
 *   \brief Test du d'un main du programme
 *   \author {Maxence.D}
 *   \version 0.1
 *   \date 10 mars 2019
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <affichage.h>
#include <biome.h>
#include <block.h>
#include <chemin.h>
#include <commun.h>
#include <couleurs.h>
#include <entite.h>
#include <fps.h>
#include <generation.h>
#include <liste.h>
#include <map.h>
#include <math.h>
#include <menu.h>
#include <outils_SDL.h>
#include <sauvegarde_map.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <touches.h>
#include <world_of_dungeons.h>

int main(int argc, char *argv[], char **env) {
  pwd_init(argv[0], getenv("PWD"));
  srand(time(NULL));
  SEED = rand() % 256;
  int i = rand() % 256;
  int taille = 30;
  t_menu *menu = NULL;
  t_map *map = NULL;
  uint8_t *ks;
  configTouches_t *ct;

  //--------------------------------------------------------------------------------------------------------------
  // Initialisation SDL
  //--------------------------------------------------------------------------------------------------------------

  if (SDL_Init(SDL_INIT_EVERYTHING) == -1) {
    printf("%s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  if (TTF_Init() == -1) {
    printf("%s\n", TTF_GetError());
    return EXIT_FAILURE;
  }

  if (IMG_Init(IMG_INIT_PNG) == -1) {
    printf("%s\n", TTF_GetError());
    return EXIT_FAILURE;
  }

  SDL_Window *screen = SDL_CreateWindow("World Of Dungeons", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width_window, height_window,
                                        SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
  SDL_GetWindowSize(screen, &width_window, &height_window);

  width_block_sdl = width_window / NB_BLOCK_WIDTH;
  height_block_sdl = height_window / NB_BLOCK_HEIGHT;

  SDL_Rect fondRect = {0, 0, width_window, height_window};
  SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);

  SDL_Texture *fond;
  Create_IMG_Texture(renderer, "./IMG/texture/fond.bmp", &fond);
  BIOME_init();
  Init_Sprite(renderer);
  BLOCK_CreateTexture_sdl(renderer);
  menu_init(renderer);
  fps_init();
  SDL_init_touches(&ks, &ct);
  // GAME_init(renderer, &ks, &ct);

  double coef_fps = 1;

  int boucle = 1;
  int x_mouse = 0, y_mouse = 0;
  t_liste affichage;
  init_liste(&affichage);
  MAP_creer(&map, "World", 12281783);

  //----------------------------------------------------------------------------------------------------------------
  // Menu Début de jeux
  //----------------------------------------------------------------------------------------------------------------

  menu_creer(MENU_PRINCIPAL, width_window, height_window, &menu);
  t_type_menu type_bouton;

#define calY_aff ((map->joueur->hitbox.y / height_block_sdl) - (MAX_SCREEN / 2)) - ((map->joueur->hitbox.h) / height_block_sdl)
#define calX_Debut ((map->joueur->hitbox.x - (map->joueur->hitbox.w / 2)) / width_block_sdl) - (SIZE / 2)
#define calX_Fin ((map->joueur->hitbox.x + (map->joueur->hitbox.w / 2)) / width_block_sdl) + (SIZE / 2)

  while (menu) {
    SDL_RenderClear(renderer);
    SDL_touches(ks, ct);
    menu_gestion_SDL(menu, SDL_touche_appuyer(ks, SOURIS_GAUCHE), &type_bouton);
    if (type_bouton == MENU_QUITTER) {
      boucle = 0;
      menu_suivant(&menu, type_bouton);
    } else if (type_bouton != MENU_NULL) {
      menu_suivant(&menu, type_bouton);
    }
    menu_afficher_SDL(menu, renderer);
    SDL_RenderPresent(renderer);
  }

  //----------------------------------------------------------------------------------------------------------------
  // Boucle de jeux
  //----------------------------------------------------------------------------------------------------------------

  while (boucle) {

    SDL_touches(ks, ct);

    //QUITTER LE JEU
    if (SDL_touche_appuyer(ks, QUITTER) || SDL_touche_appuyer(ks, ESCAPE)) {
      boucle = 0;
    }

    //GENERATION DROITE GAUCHE
    t_block *premier = MAP_GetBlockFromList(map, 0, 0);
    t_block *dernier = MAP_GetBlockFromList(map, taille_liste(map->list) - 1, 0);

    if (dernier)
      if (dernier->x < (map->joueur->hitbox.x / width_block_sdl) + SIZE / 2) {
        gen_col(map->list, (map->joueur->hitbox.x / width_block_sdl) + SIZE / 2, DROITE);
      }

    if (premier)
      if (premier->x > (map->joueur->hitbox.x / width_block_sdl) - SIZE / 2) {
        gen_col(map->list, (map->joueur->hitbox.x / width_block_sdl) - SIZE / 2, GAUCHE);
      }

    //TEST SOURIS
    t_block *b;
    if (SDL_touche_appuyer(ks, SOURIS_GAUCHE)) {
      SDL_coord_souris(&x_mouse, &y_mouse);
      // Récuperation d'un block dans la liste
      b = MAP_GetBlock(map, (x_mouse / width_block_sdl) + calX_Debut + 1, MAX_SCREEN - (y_mouse / height_block_sdl) + calY_aff);

      if (b) {
        b->id = AIR;
      } else {
        printf("Block NULL\n");
      }
    }

    if (SDL_touche_appuyer(ks, SOURIS_DROIT)) {
      SDL_coord_souris(&x_mouse, &y_mouse);
      // Récuperation d'un block dans la liste
      b = MAP_GetBlock(map, (x_mouse / width_block_sdl) + calX_Debut + 1, MAX_SCREEN - (y_mouse / height_block_sdl) + calY_aff);

      if (b) {
        b->id = ROCHE;
      } else {
        printf("Block NULL\n");
      }
    }

    taille = AFF_GetMidHeight(map->list);
    SDL_RenderCopy(renderer, fond, NULL, &fondRect);

    //On efface toute les valeurs de la liste d'affichage
    for (en_queue(&affichage); !hors_liste(&affichage); en_queue(&affichage))
      oter_elt(&affichage, NULL);

    MAP_CopyListFromX(map, &affichage, calX_Debut, calX_Fin);
    AFF_map_sdl(&affichage, renderer, calY_aff);
    printf("Affichage x_debut:%d x_fin:%d y:%d\n", calX_Debut, calX_Fin, calY_aff);

    printf("hitbox: x:%d y:%d\n", map->joueur->hitbox.x / width_block_sdl, map->joueur->hitbox.y / height_block_sdl);
    Gestion_Entite(renderer, map->joueur, ks, coef_fps, map->list);
    SDL_RenderPresent(renderer);

    coef_fps = fps();
  }
  MAP_sauvegarder(map);
  detruire_entite(map->joueur);
  Quit_Sprite();
  menu_quit();
  TTF_Quit();
  BIOME_Quit();
  MAP_detruction(&map);
  SDL_exit_touches(&ks, &ct);
  BLOCK_DestroyTexture_sdl(renderer);
  SDL_DestroyTexture(fond);
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(screen);
  SDL_Quit();
  pwd_quit();

  return 0;
}

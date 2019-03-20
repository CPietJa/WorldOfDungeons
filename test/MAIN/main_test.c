/**
 *
 *   \file main_test.c
 *   \brief Test du d'un main du programme
 *   \author {Maxence.D}
 *   \version 0.1
 *   \date 10 mars 2019
 */

#include <SDL2/SDL.h>
#include <affichage.h>
#include <biome.h>
#include <block.h>
#include <chemin.h>
#include <commun.h>
#include <couleurs.h>
#include <generation.h>
#include <liste.h>
#include <map.h>
#include <math.h>
#include <outils_SDL.h>
#include <sauvegarde_map.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <touches.h>
#include <world_of_dungeons.h>

int main(int argc, char *argv[], char **env) {
  srand(time(NULL));
  SEED = rand() % 256;
  int i = rand() % 256;
  int taille = 30;

  SDL_Init(SDL_INIT_EVERYTHING);
  SDL_Window *screen = SDL_CreateWindow("World Of Dungeons", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width_window, height_window,
                                        SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN_DESKTOP);
  SDL_GetWindowSize(screen, &width_window, &height_window);
  SDL_Rect fondRect = {0, 0, width_window, height_window};
  SDL_Renderer *renderer = SDL_CreateRenderer(screen, -1, SDL_RENDERER_ACCELERATED);
  SDL_Texture *fond;
  Create_IMG_Texture(renderer, "./IMG/texture/fond.bmp", &fond);
  BLOCK_CreateTexture_sdl(renderer);

  pwd_init(argv[0], getenv("PWD"));

  uint8_t *ks;
  configTouches_t *ct;
  SDL_init_touches(&ks, &ct);

  BIOME_init();

  t_map *map = NULL;
  MAP_creer(&map, "World", 12281783);

  int boucle = 1;
  int x_mouse = 0,y_mouse = 0;

  while(taille_liste(map->list)<= SIZE){
      i++;
      gen_col(map->list, i, DROITE);
  }

  while (boucle) {

    //taille = AFF_GetMidHeight(&list);

    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, fond, NULL, &fondRect);

    AFF_map_sdl(map->list, renderer, taille);
    SDL_RenderPresent(renderer);

    SDL_touches(ks, ct);
    if (SDL_touche_appuyer(ks, QUITTER)) {
      boucle = 0;
    } else if (SDL_touche_appuyer(ks, DROITE)) {
      i++;
      gen_col(map->list, i, DROITE);
    } else if (SDL_touche_appuyer(ks, GAUCHE)) {
      i--;
      gen_col(map->list, i - SIZE, GAUCHE);
    } else if (SDL_touche_appuyer(ks, AVANCER)) {
      taille++;
    } else if (SDL_touche_appuyer(ks, RECULER)) {
      taille--;
    } else if(SDL_touche_appuyer(ks, SOURIS_GAUCHE)){
      SDL_coord_souris(&x_mouse,&y_mouse);
      // Récuperation d'un block dans la liste
      t_block b = MAP_GetBlockFromList(map, i- (x_mouse/width_block_sdl) ,(y_mouse/height_block_sdl)+taille);

      printf("Block id: %d x:%d, y:%d x_mouse:%d y_mouse:%d\n", b.id, b.x, b.y,i - (x_mouse/width_block_sdl) ,(y_mouse/height_block_sdl)+taille);

    }

    SDL_Delay(1);
    //AFF_map_term(&list, 0, 400);
  }

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

#include <stdio.h>
#include <stdlib.h>
#include <chemin.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <outils_SDL.h>
#include <entite.h>
#include <touches.h>

int main (int argc, char ** argv, char ** env)
{
    pwd_init(argv[0],getenv("PWD"));
    SDL_Init(SDL_INIT_EVERYTHING);

    IMG_Init(IMG_INIT_PNG);

    int width = 1000;
    int height = 600;
    SDL_Window * pwindow = SDL_CreateWindow("World Of Dungeons", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,width, height, SDL_WINDOW_SHOWN);
    
    SDL_Renderer * renderer = SDL_CreateRenderer(pwindow,-1,SDL_RENDERER_ACCELERATED);

    /* Initialisation des sprites */
    Init_Sprite(renderer);

    /* Création de l'entité */
    t_entite * J = creer_entite_defaut(NULL,JOUEUR,250,600);

    uint8_t *ks;
    configTouches_t *ct;
    int continuer = 1;
    SDL_init_touches( &ks, &ct);
    while (continuer)
    {
        SDL_RenderClear(renderer);
        SDL_touches( ks, ct);

        /* Check si on doit quitter le programme */
        if (SDL_touche_appuyer( ks, QUITTER))
            continuer = 0;
        
        Gestion_Entite(renderer,J,ks);
        
        SDL_RenderPresent(renderer);
        SDL_Delay(20);
    }

    SDL_exit_touches( &ks, &ct);

    Quit_Sprite();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(pwindow);
    IMG_Quit();
    SDL_Quit();
    pwd_quit();

    return 0;
}
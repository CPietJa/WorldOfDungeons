/**
 * \file entite.c
 * \brief Module servant à gérer les entités.
 * \author GALBRUN Tibane
 * \date 13/03/2019
 * \version 0.1
*/

#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <entite.h>
#include <erreur.h>

/**
 * \fn t_entite * creer_entite_defaut (char * name, t_hitbox hitbox, SDL_Texture * texture, t_entite_type type)
 * \brief Créer une entité avec des paramètres par défaut.
 * \param name Le nom de l'entité.
 * \param hitbox La hitbox de l'entité.
 * \param texture La texture de l'entité.
 * \param type Le type de l'entité.
 * \return Un pointeur sur l'entité créée.
*/
t_entite * creer_entite_defaut (char * name, t_hitbox hitbox, SDL_Texture * texture, t_entite_type type)
{
    if (!hitbox || !texture) return NULL;
    switch (type){
        case JOUEUR : return creer_entite_defaut((name) ? name : "PLAYER", 20, 20, 10, 10, texture, hitbox);
        case ZOMBIE : return creer_entite_defaut((name) ? name : "ZOMBIE", 0, 0, 10, 10, texture, hitbox);
        case BOSS : return creer_entite_defaut((name) ? name : "BOSS", 50, 50, 30, 30, texture, hitbox);
    }
    return NULL;
}

t_entite * creer_entite (char * name, int mana, int mana_max, int pv, int pv_max, SDL_Texture * texture, t_hitbox hitbox)
{
    if (!hitbox || !texture || !name) return NULL;

    t_entite * entite = malloc(sizeof(t_entite));
    entite->id = sizeof(*texture);
    entite->name = name;
    entite->xp = 0;
    entite->faim = entite->faim_max = 10;
    entite->mana = mana;
    entite->mana_max = mana_max;
    entite->pv = pv;
    entite->pv_max = pv_max;
    entite->texture = texture;
    entite->hitbox = hitbox;

    return entite;
}

t_hitbox creer_hitbox (int x, int y, int largeur, int longueur)
{
    t_hitbox hit;
    hit.x = x;
    hit.y = y;
    hit.l = largeur;
    hiy.L = longueur;
    return hit;
}
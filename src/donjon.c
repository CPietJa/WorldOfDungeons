/**
 * \file donjon.c
 * \brief Module de création + de gestion d'un donjon
 * \author Jasmin GALBRUN
 * \version 1
 * \date 16/04/2019
*/

#include <stdlib.h>
#include <stdio.h>
#include <chemin.h>
#include <SDL2/SDL.h>
#include <liste.h>
#include <erreur.h>
#include <commun.h>
#include <donjon.h>
#include <perlin.h>
#include <block.h>
#include <outils_SDL.h>
#include <stdbool.h>
#include <affichage.h>
#include <entite.h>
#include <chemin.h>
#include <ia.h>
#include <math.h>

#define DEPTH 5
#define TAILLE_SOL 6

/* Prototypes fonctions non accessible pour l'utilisateur */
static t_erreur donjon_creer_salle(t_salle_donjon ** salle, int x, int y, t_type_salle_donjon type);
static t_erreur donjon_ajout_salle(t_liste * donjon, int taille_donjon, t_entite * joueur);
static int nb_voisin_salle(t_salle_donjon * salle);
static t_erreur selection_voisin(t_salle_donjon * salle, int * choix);
static int chercher_salle(t_liste *donjon, t_entite * joueur);
static t_erreur update_voisin(t_liste * donjon, int taille_donjon);
static t_erreur donjon_creer_structure_salle(t_salle_donjon * salle);
static void donjon_detruire_salle(t_salle_donjon *salle);
static t_erreur donjon_coord_depart(t_liste * donjon, t_salle_donjon ** salle_dep);
static t_erreur placer_salle_boss(t_liste * donjon);
static t_erreur creer_mob(t_salle_donjon * salle, t_entite * joueur);
static t_erreur salle_joueur(t_liste * donjon, SDL_Rect pos_perso, t_salle_donjon ** salle_j);
static t_erreur attaque(t_entite * attaquant, t_entite * cible);

/**
 * \fn t_erreur donjon_creer(t_donjon ** donjon, int nb_salle, t_entite * joueur)
 * \brief Fonction permettant de créer un donjon
 * \param donjon Pointeur dans lequel sera stocké le donjon
 * \param nb_salle Nombre de salle dans le donjon
 * \param joueur Joueur voulant entrer dans le donjon
 * \return Code erreur
*/
t_erreur donjon_creer(t_donjon ** donjon, int nb_salle, t_entite * joueur){
    /* Vérification */
    if(donjon == NULL){
        erreur_save(PTR_NULL, "donjon_creer() : Double poiteur sur la liste NULL");
        return PTR_NULL;
    }
    if(nb_salle <= 1){
        erreur_save(VALUE_ERROR, "donjon_creer() : Nombre de salle inferieur ou égal à 1");
        return VALUE_ERROR;
    }

    /* Création Donjon */
    *donjon = malloc(sizeof(t_donjon));

    (*donjon)->quitter = false;

    (*donjon)->donjon = malloc(sizeof(t_liste));
    init_liste((*donjon)->donjon);

    /* On sauvegarde les coordonnées du joueur avant d'entrer dans le donjon */
    (*donjon)->x_map_joueur = joueur->hitbox.x;
    (*donjon)->y_map_joueur = joueur->hitbox.y;

    /* Calcul taille donjon */
    int taille_donjon = nb_salle;

    /* Création Structure donjon */
    while(nb_salle--){
        donjon_ajout_salle((*donjon)->donjon, taille_donjon, joueur);
    }

    /* Placement de la salle du boss */
    placer_salle_boss((*donjon)->donjon);

    /* Création structure salle */
    t_salle_donjon * salle = NULL;
    for(en_tete((*donjon)->donjon); !hors_liste((*donjon)->donjon); suivant((*donjon)->donjon)){
        valeur_elt((*donjon)->donjon, (void **)&salle);
        donjon_creer_structure_salle(salle);
    }

    /* Placement du joueur au début du donjon */
    salle = NULL;
    donjon_coord_depart((*donjon)->donjon, &salle);
    joueur->hitbox.x = (salle->x * SIZE + (SIZE / 3)) * width_block_sdl;
    joueur->hitbox.y = (salle->y * MAX_SCREEN + (TAILLE_SOL)) * height_block_sdl + joueur->hitbox.h;

    return OK;
}

/**
 * \fn static t_erreur donjon_ajout_salle(t_liste * donjon, int taille_donjon, t_entite * joueur)
 * \brief Fonction d'ajout d'une salle à un donjon
 * \param donjon Donjon dans lequel on veut ajouter la salle
 * \param taille_donjon Taille du donjon
 * \param joueur Joueur qui veut entrer dans le donjon
 * \return Code erreur
*/
static t_erreur donjon_ajout_salle(t_liste * donjon, int taille_donjon, t_entite * joueur){
    /* Vérification */
    if(donjon == NULL){
        erreur_save(PTR_NULL, "donjon_ajout_salle() : Poiteur sur la liste NULL");
        return PTR_NULL;
    }

    /* Création salle */
    t_salle_donjon * salle = NULL;

    if(liste_vide(donjon)){
        donjon_creer_salle(&salle, taille_donjon / 2, taille_donjon / 2, DONJON_DEPART);
        ajout_droit(donjon, salle);
        return OK;
    }

    /* On sélectionne une salle */
    int i = chercher_salle(donjon, joueur);
    t_salle_donjon * tamp;
    valeur_liste(donjon, i, (void **)&tamp);

    /* On sélectionne une case voisine */
    int choix;
    selection_voisin(tamp, &choix);

    int x = tamp->x;
    int y = tamp->y;
    if(choix == 0){
        x--;
    }else if(choix == 1){
        y++;
    }else if(choix == 2){
        x++;
    }else if(choix == 3){
        y--;
    }
    
    /* On crée la salle puis on l'ajoute à la liste */
    donjon_creer_salle(&salle, x, y, DONJON_INTERMEDIAIRE);
    en_queue(donjon);
    ajout_droit(donjon, salle);
    update_voisin(donjon, taille_donjon);

    return OK;
}

/**
 * \fn static t_erreur donjon_creer_salle(t_salle_donjon ** salle, int x, int y, t_type_salle_donjon type)
 * \brief Fonction qui crée une salle en remplissant ces champs par défault
 * \param salle Pointeur dans lequel on enregistre la salle créé
 * \param x Position en X de la salle dans le donjon
 * \param y Position en Y de la salle dans le donjon
 * \param type Type de la salle
 * \return Code erreur
*/
static t_erreur donjon_creer_salle(t_salle_donjon ** salle, int x, int y, t_type_salle_donjon type){
    /* Vérification */
    if(salle == NULL){
        erreur_save(PTR_NULL, "donjon_creer_salle() : Doule pointeur sur la salle NULL");
        return PTR_NULL;
    }

    /* Création salle */
    *salle = malloc(sizeof(t_salle_donjon));
    (*salle)->x = x;
    (*salle)->y = y;
    int i;
    for(i = 0; i < 4; i++){
        (*salle)->voisin[i] = 0;
    }

    (*salle)->type = type;
    (*salle)->completed = false;
    (*salle)->mob = NULL;

    /* On sélectionne une difficulté */
    if(type == DONJON_DEPART){
        (*salle)->completed = true;
        (*salle)->difficulte = AUCUNE;
    }else if(type == DONJON_FIN){
        (*salle)->difficulte = FINAL;
    }else{
        
        int alea = noise2(x, y) % 3;
        
        if(alea == 0)
            (*salle)->difficulte = FACILE;
        else if(alea == 1)
            (*salle)->difficulte = MOYEN;
        else
            (*salle)->difficulte = DIFFICILE;
    }

    return OK;
}

/**
 * \fn static int nb_voisin_salle(t_salle_donjon * salle)
 * \brief Calcule le nombre de voisin d'une salle
 * \param salle Salle dont on veut connaitre le nombre de voisin
 * \return Nombre de voisins de la salle
*/
static int nb_voisin_salle(t_salle_donjon * salle){
    int nb = 0;
    if(salle == NULL)
        return nb;
    int i;
    for(i = 0; i < 4; i++)
        if(salle->voisin[i] == 0)
            nb++;
    
    return nb;
}

/**
 * \fn static t_erreur selection_voisin(t_salle_donjon * salle, int * choix)
 * \brief Fonction qui sélectionne de manière pseudo-aléatoire un voisin pour une salle donnée
 * \param salle Salle dont on veut sélectionner un voisin
 * \param choix Pointeur de retour sur le choix fait
 * \return Code erreur
*/
static t_erreur selection_voisin(t_salle_donjon * salle, int * choix){
    /* Vérification */
    if(salle == NULL){
        erreur_save(PTR_NULL, "selection_voisin() : Pointeur sur salle NULL");
        return PTR_NULL;
    }
    if(choix == NULL){
        erreur_save(PTR_NULL, "selection_voisin() : Pointeur sur choix NULL");
        return PTR_NULL;
    }

    /* On sélectionne une salle inexistante */
    t_liste * liste = malloc(sizeof(t_liste));
    init_liste(liste);
    int i;
    for(i = 0; i < 4; i++){
        if(salle->voisin[i] == 0){
            int * tamp = malloc(sizeof(int));
            *tamp = i;
            ajout_droit(liste, (void *)tamp);
        }
    }

    int choix_voisin;
    choix_voisin = noise2(salle->x, salle->y) % taille_liste(liste);
    int *choix_inter;
    valeur_liste(liste, choix_voisin, (void **)&choix_inter);
    *choix = *choix_inter;

    /* On libère la mémoire */
    detruire_liste(liste, free);
    free(liste);
    
    return OK;
}

/**
 * \fn static int chercher_salle(t_liste *donjon, t_entite * joueur)
 * \brief Cherche une salle ayant au moins 1 voisin (Slot dans le quelle une autre salle peut être créer)
 * \param donjon Donjon dans lequel on recherche la salle
 * \param joueur Jour qui veut entrer dans le donjon
 * \return La position de la salle dans la liste, -1 sinon
*/
static int chercher_salle(t_liste *donjon, t_entite * joueur){
    /* On sélectionne une salle aléatoire */
    int taille_l = taille_liste(donjon);
    
    int i = noise2(joueur->hitbox.x, joueur->hitbox.y) % taille_l;
    
    /* On cherche une salle ayant au moins 1 voisin */
    t_salle_donjon * salle;
    int verif = 0;
    while(!verif){
        valeur_liste(donjon, i, (void **)&salle);

        if(nb_voisin_salle(salle) <= 0){
            if(i >= taille_l - 1)
                i = 0;
            else
                i++;            
        }else{
            verif = 1;
        }
    }
    return i;
}

/**
 * \fn static t_erreur update_voisin(t_liste * donjon, int taille_donjon)
 * \brief Met à jour le tableau des voisins de chaque salle
 * \param donjon Donjon dans lequel on veut mettre à jour les salles
 * \param taille_donjon Taille du donjon
 * \return Code erreur
*/
static t_erreur update_voisin(t_liste * donjon, int taille_donjon){
    /* Vérification */
    if(donjon == NULL){
        erreur_save(PTR_NULL, "update_voisin() : Pointeur sur donjon NULL");
        return PTR_NULL;
    }

    /* Mise à jour des voisins */
    t_salle_donjon *salle_courante, *salle_suivante;

    int i;
    for(i = 0; i < taille_liste(donjon); i++){
        valeur_liste(donjon, i, (void **)&salle_courante);

        for(en_tete(donjon); !hors_liste(donjon); suivant(donjon)){
            valeur_elt(donjon, (void **)&salle_suivante);
            if(salle_courante != salle_suivante){
                if(salle_courante->y == salle_suivante->y){
                    if(salle_courante->x - 1 == salle_suivante->x){
                        salle_courante->voisin[0] = 1;
                        salle_suivante->voisin[2] = 1;
                    }else if(salle_courante->x + 1 == salle_suivante->x){
                        salle_courante->voisin[2] = 1;
                        salle_suivante->voisin[0] = 1;
                    }
                }else if(salle_courante->x == salle_suivante->x){
                    if(salle_courante->y - 1 == salle_suivante->y){
                        salle_courante->voisin[3] = 1;
                        salle_suivante->voisin[1] = 1;
                    }else if(salle_courante->y + 1 == salle_suivante->y){
                        salle_courante->voisin[1] = 1;
                        salle_suivante->voisin[3] = 1;
                    }
                }
            }
            /* Bordure du donjon */
            if(salle_courante->x == 0){
                salle_courante->voisin[0] = 1;
            }else if(salle_courante->x == taille_donjon - 1){
                salle_courante->voisin[2] = 1;
            }else if(salle_courante->y == 0){
                salle_courante->voisin[3] = 1;
            }else if(salle_courante->y == taille_donjon - 1){
                salle_courante->voisin[1] = 1;
            }
        }
    }

    return OK;
}

/**
 * \fn static t_erreur donjon_creer_structure_salle(t_salle_donjon * salle)
 * \brief Crée la structure de la salle
 * \param salle Salle dans laquelle on veut créer une structure
 * \return Code erreur
*/
static t_erreur donjon_creer_structure_salle(t_salle_donjon * salle){
    /* Vérification */
    if(salle == NULL){
        erreur_save(PTR_NULL, "donjon_creer_structure_salle() : Pointeur sur salle NULL");
        return PTR_NULL;
    }

    /* Initialisation structure salle */
    t_liste * liste = malloc(sizeof(t_liste));
    init_liste(liste);
    salle->structure = liste;

    /* Création structure salle */
    t_block * tab = NULL;
    int i;
    for(i = 0; i < SIZE; i++){
        /* Génération d'une colonne */
        tab = malloc(sizeof(t_block) * MAX_SCREEN);
        int hauteur;

        /* Génération relief plafond */
        hauteur = perlin2d(i, salle->x * salle->y, FREQ * 5, DEPTH) * (MAX_SCREEN) + (MAX_SCREEN / 4);
        if(hauteur >= MAX_SCREEN - 1)
            hauteur = MAX_SCREEN - 1;
        
        int j;
        for(j = 0; j < MAX_SCREEN; j++){
            tab[j].x = salle->x * SIZE + i;
            tab[j].y = salle->y * MAX_SCREEN + j;

            
            /* Trou échelle Plafond */
            if(j >= hauteur && (i == SIZE / 2 || i == SIZE / 2 + 1) && salle->voisin[1] == 1){
                tab[j].id = AIR;
            }
            /* Trou échelle sol */
            else if(j <= TAILLE_SOL && (i == SIZE / 2 || i == SIZE / 2 + 1) && salle->voisin[3] == 1){
                tab[j].id = AIR;
            }
            /* Trou mur gauche */
            else if(j < MAX_SCREEN / 1.7 && j > TAILLE_SOL && i == SIZE - 1 && salle->voisin[2] == 1){
                tab[j].id = AIR;
            }
            /* Trou mur droite */
            else if(j < MAX_SCREEN / 1.7 && j > TAILLE_SOL && i == 0 && salle->voisin[0] == 1){
                tab[j].id = AIR;
            }
            /* Le reste des murs */
            else if(j >= hauteur || j <= TAILLE_SOL || i == 0 || i == SIZE - 1){
                tab[j].id = ROCHE;
            }else{
                tab[j].id = AIR;
            }

            if(tab[j].id == AIR)
                tab[j].plan = ARRIERE_PLAN;
            else
                tab[j].plan = PREMIER_PLAN;
        }
        en_queue(liste);
        ajout_droit(liste, tab);
    }

    return OK;
}

/**
 * \fn static t_erreur donjon_coord_depart(t_liste * donjon, t_salle_donjon ** salle_dep)
 * \brief Renvoie la salle de départ
 * \param donjon Donjon dont on veut connaitre la salle de départ
 * \param salle_dep Double pointeur de retour de la salle de départ
 * \return Code erreur 
*/
static t_erreur donjon_coord_depart(t_liste * donjon, t_salle_donjon ** salle_dep){
    /* Vérification */
    if(donjon == NULL){
        return PTR_NULL;
    }
    if(salle_dep == NULL){
        return PTR_NULL;
    }

    /* On cherche le départ */
    t_salle_donjon * salle = NULL;
    for(en_tete(donjon); !hors_liste(donjon) && (salle == NULL || salle->type != DONJON_DEPART); suivant(donjon)){
        valeur_elt(donjon, (void**)&salle);
    }

    /* On met à jour les coordonnées */
    *salle_dep = salle;
    
    return OK;
}

/**
 * \fn static t_erreur placer_salle_boss(t_liste * donjon)
 * \brief Place la salle du boss dans la salle la plus éloigné de la salle de départ
 * \param donjon Donjon dans lequel on veut placer la salle du boss
 * \return Code erreur
*/
static t_erreur placer_salle_boss(t_liste * donjon){
    /* Vérification */
    if(donjon == NULL){
        return PTR_NULL;
    }

    /* On sélectionne la salle la plus loin de la salle de départ */
    t_salle_donjon * salle_dep = NULL;
    donjon_coord_depart(donjon, &salle_dep);

    t_salle_donjon * salle_max = salle_dep;
    t_salle_donjon * salle = NULL;
    for(en_tete(donjon); !hors_liste(donjon); suivant(donjon)){
        valeur_elt(donjon, (void**)&salle);
        int norme = (salle->x - salle_dep->x) * (salle->y - salle_dep->y);
        int norme2 = (salle_max->x - salle_dep->x) * (salle_max->y - salle_dep->y);
        //fprintf(stderr, "salle: x=%d, y=%d | max: x=%d, y=%d | dep: x=%d, y=%d\n", salle->x, salle->y, salle_max->x, salle_max->y, salle_dep->x, salle_dep->y);
        if(norme > norme2){
            salle_max = salle;
        }
    }
    
    salle_max->type = DONJON_FIN;
    salle_max->difficulte = FINAL;
    
    return OK;
}

/**
 * \fn static t_erreur salle_joueur(t_liste * donjon, SDL_Rect pos_perso, t_salle_donjon ** salle_j)
 * \brief Retourne dans tab la partie du donjon à afficher en fonction du joueur
 * \param donjon Donjon que l'on veut afficher
 * \param pos_perso Position du joueur
 * \param salle_j Double pointeur de retour de la salle dans laquelle se trouve le joueur
 * \return Code erreur
*/
static t_erreur salle_joueur(t_liste * donjon, SDL_Rect pos_perso, t_salle_donjon ** salle_j){
    /* Vérification */
    if(donjon == NULL){
        erreur_save(PTR_NULL, "salle_joueur() : Pointeur sur donjon NULL");
        return PTR_NULL;
    }
    if(salle_j == NULL){
        erreur_save(PTR_NULL, "salle_joueur() : Double pointeur sur salle_j NULL");
        return PTR_NULL;
    }

    //Coordonnées de la salle où se trouve le joueur
    int x_salle = (float)(pos_perso.x / width_block_sdl) / SIZE;
    int y_salle = (float)(pos_perso.y / height_block_sdl) / MAX_SCREEN;
    
    //On cherche la salle dans le donjon
    t_salle_donjon * salle = NULL;
    for(en_tete(donjon); !hors_liste(donjon) && (salle == NULL || (salle->x != x_salle || salle->y != y_salle)); suivant(donjon)){
        valeur_elt(donjon, (void**)&salle);
    }

    *salle_j = salle;
    
    return OK;
}

/**
 * \fn t_erreur donjon_afficher_Term(t_donjon * donjon, t_entite * joueur)
 * \brief Fonction d'affichage du donjon dans un terminal
 * \param donjon Donjon que l'on veut afficher
 * \param joueur Joueur qui est dans le donjon
 * \return Code erreur
*/
t_erreur donjon_afficher_Term(t_donjon * donjon, t_entite * joueur){
    /* Vérification */
    if(donjon == NULL){
        erreur_save(PTR_NULL, "donjon_afficher_Term() : Pointeur sur donjon NULL");
        return PTR_NULL;
    }
    if(donjon->quitter == true){
        return OK;
    }

    /* Affichage du donjon */
    t_salle_donjon * salle = NULL;

    salle_joueur(donjon->donjon, joueur->hitbox, &salle);

    AFF_map_term(salle->structure, 0, SIZE-1);

    return OK;
}

/**
 * \fn t_erreur donjon_afficher_SDL(SDL_Renderer * renderer, t_donjon * donjon, t_entite * joueur)
 * \brief Fonction d'affichage d'un donjon avec la SDL
 * \param renderer Rendu de la fenêtre
 * \param donjon Donjon que l'on veut afficher
 * \param joueur Joueur qui est dans le donjon
*/
t_erreur donjon_afficher_SDL(SDL_Renderer * renderer, t_donjon * donjon, t_entite * joueur){
    /* Vérification */
    if(donjon == NULL){
        erreur_save(PTR_NULL, "donjon_afficher_SDL() : Pointeur sur donjon NULL");
        return PTR_NULL;
    }
    if(renderer == NULL){
        erreur_save(PTR_NULL, "donjon_afficher_SDL() : Pointeur sur renderer NULL");
        return PTR_NULL;
    }
    if(donjon->quitter == true){
        return OK;
    }

    /* Affichage du donjon */
    t_salle_donjon * salle = NULL;

    salle_joueur(donjon->donjon, joueur->hitbox, &salle);

    // AFF_map_sdl(fenetre, renderer, -1);
    int i, j;
    for(j = 0, en_tete(salle->structure); !hors_liste(salle->structure); suivant(salle->structure), j++){
        SDL_Texture * texture = NULL;
        t_block * colonne = NULL;
        valeur_elt(salle->structure, (void**)&colonne);
        for(i = 0; i < MAX_SCREEN; i++){
            texture = BLOCK_GetTexture_sdl(colonne[i].id);
            SDL_Rect r = {
                j * width_block_sdl,
                (MAX_SCREEN - 1 - i) * height_block_sdl,
                width_block_sdl,
                height_block_sdl
            };
            SDL_RenderCopy(renderer, texture, NULL, &r);
        }
    }

    return OK;
}

/**
 * \fn t_erreur donjon_gestion(SDL_Renderer * renderer, t_donjon * donjon, t_entite * joueur, uint8_t *ks, double coef_fps)
 * \brief Permet de gérer un donjon comme la création et l'interaction des mobs
 * \param renderer Rendu de la fenêtre
 * \param donjon Donjon que l'on veut gérer
 * \param joueur Joueur qui est dans le donjon
 * \param ks Pointeur sur l'état des touches
 * \param coef_fps Coefficient de retard de fps
 * \return Code erreur
*/
t_erreur donjon_gestion(SDL_Renderer * renderer, t_donjon * donjon, t_entite * joueur, uint8_t *ks, double coef_fps){
    /* Vérification */
    if(renderer == NULL){
        erreur_save(PTR_NULL, "donjon_gestion() : Pointeur sur renderer NULL");
        return PTR_NULL;
    }
    if(donjon == NULL){
        erreur_save(PTR_NULL, "donjon_gestion() : Pointeur sur donjon NULL");
        return PTR_NULL;
    }
    if(joueur == NULL){
        erreur_save(PTR_NULL, "donjon_gestion() : Pointeur sur joueur NULL");
        return PTR_NULL;
    }
    if(donjon->quitter == true){
        return OK;
    }

    /* Initialisation */
    t_salle_donjon * salle = NULL;
    salle_joueur(donjon->donjon, joueur->hitbox, &salle);
    t_entite * ref = creer_entite_defaut("ref", JOUEUR, (salle->x * SIZE) + (SIZE / 2), (salle->y * MAX_SCREEN) + (MAX_SCREEN / 2), 2 * height_block_sdl);

    /* Gestion Joueur */
    Gestion_Entite(renderer, joueur, ks, coef_fps, salle->structure, GESTION_TOUCHES, ALL_ACTION, ref, NOT_CENTER_SCREEN);

    
    /* Gestion mob */
    if(salle->completed == false){

        /* Création mob */
        if(salle->mob == NULL){
            salle->mob = malloc(sizeof(t_liste));
            init_liste(salle->mob);

            creer_mob(salle, joueur);
        }

        t_entite * mob = NULL;
        t_action action;

        /* Mob jouer */
        for(en_tete(salle->mob); !hors_liste(salle->mob); suivant(salle->mob)){
            valeur_elt(salle->mob, (void**)&mob);

            if(mob->pv > 0 || mob->type == BOSS){
                int x_salle_mob = (mob->hitbox.x / width_block_sdl) / SIZE;
                
                if(x_salle_mob < salle->x){
                    action = MARCHE_DROITE;
                }else if(x_salle_mob > salle->x){
                    action = MARCHE_GAUCHE;
                }else{
                    action = ia_jouer(mob, joueur, IA_ALEATOIRE);
                }
                attaque(joueur, mob);
                attaque(mob, joueur);

                update_posY_entite(mob, coef_fps, salle->structure, NOT_CENTER_SCREEN);
                Gestion_Entite(renderer, mob, ks, coef_fps, salle->structure, GESTION_ACTION, action, ref, NOT_CENTER_SCREEN);
            }else{
                salle->completed = true;
            }
        }
    }else if(salle->type == DONJON_FIN){
        donjon->quitter = true;
    }

    /* Libération Mémoire */
    detruire_entite(ref);

    return OK;
}

/**
 * \fn static t_erreur creer_mob(t_salle_donjon * salle, t_entite * joueur)
 * \brief Fonction de création de mobs par rapport à une salle
 * \param salle Salle dans laquelle on veut créer les mobs
 * \param joueur Joueur qui est dans le donjon
 * \return Code erreur
*/
static t_erreur creer_mob(t_salle_donjon * salle, t_entite * joueur){
    /* Vérification */
    if(salle == NULL){
        erreur_save(PTR_NULL, "creer_mob() : Pointeur sur salle NULL");
        return PTR_NULL;
    }
    if(salle->mob == NULL){
        erreur_save(PTR_NULL, "creer_mob() : Pointeur sur salle->mob NULL");
        return PTR_NULL;
    }
    if(joueur == NULL){
        erreur_save(PTR_NULL, "creer_mob() : Pointeur sur joueur NULL");
        return PTR_NULL;
    }

    /* Initialisation */
    int nb_mob = 0;
    float coef;
    t_entite * mob = NULL;
    
    if(salle->difficulte == AUCUNE){
        coef = 0.0;
    }else if(salle->difficulte == FACILE)
        coef = 1.0;
    else if(salle->difficulte == MOYEN)
        coef = 1.5;
    else if(salle->difficulte == DIFFICILE)
        coef = 2.0;
    else
        coef = 2.5;

    
    nb_mob = rand() % 1 + 3;   
    nb_mob *= coef;

    int taille_mob = 4;
    int taille_boss = 6;
    int posX_mob = salle->x * SIZE + SIZE / 3;
    int posY_mob = (salle->y * MAX_SCREEN) + TAILLE_SOL + 2;
    
    while(nb_mob--){
        mob = creer_entite_defaut("Mob", ZOMBIE, posX_mob, posY_mob + taille_mob, taille_mob * height_block_sdl);
        Change_Damage_Entite(mob, 7);
        Change_PV_Entite(mob, 50, 50);
        en_queue(salle->mob);
        ajout_droit(salle->mob, (void *)mob);
    }

    if(salle->type == DONJON_FIN){
        mob = creer_entite_defaut("BOSS", BOSS, posX_mob, posY_mob + taille_boss, taille_boss * height_block_sdl);
        Change_Damage_Entite(mob, 25);
        Change_PV_Entite(mob, 150, 150);
        en_queue(salle->mob);
        ajout_droit(salle->mob, (void *)mob);
    }

    return OK;
}

/**
 * \fn static t_erreur attaque(t_entite * attaquant, t_entite * cible)
 * \brief Fonction d'attaque d'une entité sur une autre
 * \param attaquant Entité qui veut attaquer
 * \param cible Entité ciblé par l'attaque
 * \return Code Erreur
*/
static t_erreur attaque(t_entite * attaquant, t_entite * cible){
    /* Vérification */
    if(attaquant == NULL){
        erreur_save(PTR_NULL, "attaque() : Pointeur sur attaquant NULL");
        return PTR_NULL;
    }
    if(cible == NULL){
        erreur_save(PTR_NULL, "attaque() : Pointeur sur attaquant NULL");
        return PTR_NULL;
    }

    if(attaquant->type == JOUEUR){
        int mouseX, mouseY;
        SDL_GetMouseState(&mouseX, &mouseY);
        SDL_Point p = {
            mouseX,
            mouseY
        };

        if(SDL_PointInRect(&p, &(cible->posEnt))){
            if(abs(attaquant->hitbox.x - cible->hitbox.x) <= 2 * width_block_sdl){
                if(abs(attaquant->hitbox.y - cible->hitbox.y) <= 2 * height_block_sdl){
                    Add_PV_Entite(cible, -(attaquant->damage));
                }
            }
        }
    }else{
        if(attaquant->act_pred == ATTAQUE_DROITE && cible->hitbox.x >= attaquant->hitbox.x){
            Add_PV_Entite(cible, -(attaquant->damage));
        }else if(attaquant->act_pred == ATTAQUE_GAUCHE && cible->hitbox.x <= attaquant->hitbox.x){
            Add_PV_Entite(cible, -(attaquant->damage));
        }
    }
}

/**
 * \fn t_erreur donjon_quitter(t_donjon * donjon, t_entite * joueur)
 * \brief Fonction pour quitter un donjon
 * \param donjon Donjon que l'on veut quitter
 * \param joueur Joueur qui veut quitter le donjon
 * \return Code erreur
*/
t_erreur donjon_quitter(t_donjon * donjon, t_entite * joueur){
    /* Vérification */
    if(donjon == NULL){
        erreur_save(PTR_NULL, "donjon_quitter() : Pointeur sur donjon NULL");
        return PTR_NULL;
    }
    if(joueur == NULL){
        erreur_save(PTR_NULL, "donjon_quitter() : Pointeur sur joueur NULL");
        return PTR_NULL;
    }
    if(donjon->quitter == false){
        return OK;
    }

    joueur->hitbox.x = donjon->x_map_joueur;
    joueur->hitbox.y = donjon->y_map_joueur;

    return donjon_detruire(&donjon);
}

/**
 * \fn t_erreur donjon_detruire(t_donjon **donjon)
 * \brief Fonction de destruction d'un donjon
 * \param donjon Donjon que l'on veut détruire
 * \return Code erreur
*/
t_erreur donjon_detruire(t_donjon **donjon){
    /* Vérification */
    if(donjon == NULL){
        erreur_save(PTR_NULL, "donjon_detruire() : Double pointeur sur liste NULL");
        return PTR_NULL;
    }
    if(*donjon == NULL){
        return OK;
    }

    /* Destruction donjon */
    detruire_liste((*donjon)->donjon, (void *)donjon_detruire_salle);
    
    if((*donjon)->donjon != NULL){
        free((*donjon)->donjon);
        (*donjon)->donjon = NULL;
    }

    free(*donjon);
    *donjon = NULL;

    return OK;
}

/**
 * \fn static void donjon_detruire_salle(t_salle_donjon *salle)
 * \brief Fonction de destruction d'une salle
 * \param salle Salle que l'on veut détruire
*/
static void donjon_detruire_salle(t_salle_donjon *salle){
    if(salle != NULL){
        if(salle->structure != NULL){
            /* Destruction structure salle */
            detruire_liste(salle->structure, free);
            free(salle->structure);
            salle->structure = NULL;
        }
        if(salle->mob != NULL){
            /* Destruction mobs */
            detruire_liste(salle->mob, (void*)detruire_entite);
            free(salle->mob);
            salle->mob = NULL;
        }
        free(salle);
    }
}
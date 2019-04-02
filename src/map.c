/**
 *   \file map.c
 *   \brief Module permettant de géré une map (Création,Suppression)
 *   \author {Maxence.D}
 *   \version 0.1
 *   \date 07 mars 2019
 **/

#include <block.h>
#include <commun.h>
#include <erreur.h>
#include <liste.h>

#include <affichage.h>
#include <binaire.h>
#include <generation.h>
#include <json.h>
#include <map.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <touches.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <unistd.h>

/**
    \fn t_erreur MAP_creer(t_map ** map, char * nom_map, int SEED)
    \brief Créer une map (le dossier de saugarde et le pointeur pour manipuler cette map)
    \param map le pointeur de la map qui va être créer
    \param nom_map Le nom de la map
    \param SEED Le SEED de génération de la map
    \return Renvoie un code erreur en cas de problème sinon OK
**/
t_erreur MAP_creer(t_map **map, char *nom_map, int SEED) {

  (*map) = malloc(sizeof(t_map));
  (*map)->nom = malloc(sizeof(char) * strlen(nom_map) + 1);

  strcpy((*map)->nom, nom_map);
  (*map)->SEED = SEED;

  MAP_creer_dir(*map);

  (*map)->list = malloc(sizeof(t_liste));
  init_liste((*map)->list);

  int i = 20000;
  while (taille_liste((*map)->list) <= SIZE) {
    i++;
    gen_col((*map)->list, i, DROITE);
  }
  t_block *b = MAP_GetBlockFromList((*map), SIZE / 2, AFF_GetMidHeight((*map)->list));
  (*map)->joueur = creer_entite_defaut("Virgile", JOUEUR, b->x, b->y, width_block_sdl * 2);

  MAP_sauvegarder(*map);
  return OK;
}

/**
    \fn t_erreur MAP_charger(t_map ** map, char * nom_map)
    \brief Charge la map a partir des fichier
    \param map Pointeur sur la map qui doit etre sauvegarder
    \return Renvoie un code erreur en cas de problème sinon OK
**/
t_erreur MAP_charger(t_map **map, char *nom_map) {
  char *path_dir = MAP_creer_path(nom_map);
  char *path_player = malloc(strlen(path_dir) * sizeof(char) + strlen("player/") * sizeof(char));
  strcpy(path_player, path_dir);
  strcat(path_player, "player/");

  int size = 500;

  char *dir_curr = malloc(sizeof(char) * size + 1);
  getcwd(dir_curr, size);

  if (chdir(path_dir))
    return FILE_ERROR; // DIR_NO_FOUND

  chdir(dir_curr);
  free(dir_curr);

  (*map) = malloc(sizeof(t_map));
  (*map)->nom = malloc(sizeof(char) * strlen(nom_map) + 1);
  strcpy((*map)->nom, nom_map);

  // Charger le seed avec fct json
  FILE *data = open_json(path_dir, "data", "r");
  char *objet;

  fstart(data);
  extract_json_obj(data, &objet);
  read_json_obj(objet, "SEED", &(*map)->SEED, "d");
  fclose(data);
  free(objet);

  (*map)->joueur = Load_Entite(path_player, "player");
  //Charger map a partir d'un fichier
  (*map)->list = malloc(sizeof(t_liste));
  init_liste((*map)->list);

  t_binaire bin_map = Open_BIN(path_dir, (*map)->nom, "r");
  t_block *b = malloc(sizeof(t_block) * MAX);
  while (bin_map && Read_BIN(b, sizeof(t_block), MAX, bin_map) == OK) {
    ajout_droit((*map)->list, b);
    b = malloc(sizeof(t_block) * MAX);
  }
  Close_BIN(bin_map);

  MAP_detruire_path(&path_dir); // Gestion des erreurs a faire
  free(path_player);
  return OK;
}

/**
    \fn t_erreur MAP_lister()
    \brief Liste toute les map
    \param map Pointeur sur la map qui doit etre sauvegarder
    \return Renvoie un code erreur en cas de problème sinon OK
**/
t_erreur MAP_lister() {
  ;
  ;
  return OK;
}

/**
    \fn t_erreur MAP_sauvegarder(t_map * map)
    \brief Sauvegarde la map dans l'état actuel
    \param map Pointeur sur la map qui doit etre sauvegarder
    \return Renvoie un code erreur en cas de problème sinon OK
**/
t_erreur MAP_sauvegarder(t_map *map) {
  if (map == NULL)
    return PTR_NULL;
  if (map->joueur == NULL)
    return PTR_NULL;

  char *path_dir = MAP_creer_path(map->nom);
  char *path_player = malloc(strlen(path_dir) * sizeof(char) + strlen("player/") * sizeof(char));
  strcpy(path_player, path_dir);
  strcat(path_player, "player/");

  FILE *data = open_json(path_dir, "data", "w");
  open_json_obj(data);
  write_json(data, "SEED", (void *)&(map->SEED), "d");
  close_json_obj(data);
  fclose(data);

  Save_Entite(map->joueur, path_player, "player");

  t_binaire bin_map = Open_BIN(path_dir, map->nom, "w");
  t_block *b;
  for (en_tete(map->list); !hors_liste(map->list); suivant(map->list)) {
    valeur_elt(map->list, (void **)&b);
    Write_BIN(b, sizeof(t_block), MAX, bin_map);
  }
  Close_BIN(bin_map);

  MAP_detruire_path(&path_dir); // Gestion des erreurs a faire
  free(path_player);
  return OK;
}

/**
    \fn t_erreur MAP_creer_dir(t_map * map)
    \brief Créer le repertoire d'une map
    \param map Pointeur sur la map dont le repertoire doit être créer
    \return Renvoie un code erreur en cas de problème sinon OK
**/
t_erreur MAP_creer_dir(t_map *map) {
  if (map == NULL)
    return PTR_NULL;
  t_erreur erreur;

  char *path_dir = MAP_creer_path(map->nom);
  char *path_player = malloc(strlen(path_dir) * sizeof(char) + strlen("player/") * sizeof(char));
  strcpy(path_player, path_dir);
  strcat(path_player, "player/");

  if (mkdir(path_dir, S_IRWXU | S_IRWXG | S_IROTH))
    return FILE_ERROR; // MKDIR_ERROR
  if (mkdir(path_player, S_IRWXU | S_IRWXG | S_IROTH))
    return FILE_ERROR; // MKDIR_ERROR

  erreur = MAP_detruire_path(&path_dir);
  if (erreur != OK)
    return erreur;
  free(path_player);

  return OK;
}

/**
    \fn t_erreur MAP_supprimer_dir(t_map * map)
    \brief Supprimer le repertoire d'une map
    \param map Pointeur sur la map dont le repertoire doit être supprimer
    \return Renvoie un code erreur en cas de problème sinon OK
**/
t_erreur MAP_supprimer(t_map *map) {
  if (map == NULL)
    return PTR_NULL;

  char *path_dir = MAP_creer_path(map->nom);

  rmdir(path_dir);

  MAP_detruire_path(&path_dir);

  return OK;
}

/**
    \fn t_erreur MAP_supprimer(t_map ** map)
    \brief Supprime un pointeur t_map
    \param map Pointeur de pointeur de t_map à supprimer et mettre a NULL
    \return Renvoie un code erreur en cas de problème sinon OK
**/
t_erreur MAP_detruction(t_map **map) {
  if (map == NULL)
    return PTR_NULL;

  if ((*map)->nom != NULL) {
    free((*map)->nom);
    (*map)->nom = NULL;
  }
  detruire_liste((*map)->list, free);
  free((*map)->list);
  (*map)->list = NULL;
  free(*map);
  map = NULL;
  return OK;
}

/**
    \fn char * MAP_path(t_map * map)
    \brief Créer le path d'une map
    \param map Pointeur de t_map
    \return Renvoie le path de la map
**/
char *MAP_creer_path(char *nom_map) {
  char *path_dir = malloc(sizeof(char) * strlen(nom_map) + sizeof(char) * 500); //Utilise PWD pour éviter de malloc 500 sizeof char
  strcpy(path_dir, PATH_MAP_DIR);
  strcat(path_dir, nom_map);
  strcat(path_dir, "/");
  return path_dir;
}

/**
    \fn t_erreur MAP_detruire_path(char ** path_dir)
    \brief Libère la mémoire dynamique alloué pour le path
    \param map Pointeur de pointeur sur le path
    \return Renvoie un code erreur en cas de problème sinon OK
**/
t_erreur MAP_detruire_path(char **path_dir) {
  if (*path_dir == NULL)
    return PTR_NULL;

  free(*path_dir);
  path_dir = NULL;
  return OK;
}

t_block *MAP_GetBlockFromList(t_map *map, int x, int y) {
  if (y >= 0 && y < MAX) {
    t_block *tab;
    t_erreur err = valeur_liste(map->list, x, (void **)&tab);
    if (err == OK) {
      return &tab[y];
    }
  }
  return NULL;
}

t_block *MAP_GetBlock(t_map *map, int x, int y) {
  if (y >= 0 && y < MAX) {
    t_block *tab;
    for (en_tete(map->list); !hors_liste(map->list); suivant(map->list)) {
      valeur_elt(map->list, (void **)&tab);
      if (tab[0].x == x)
        return &tab[y];
    }
  }
  return NULL;
}

void MAP_SetEcListe(t_liste *list, int x) {
  t_block *b = NULL;
  for (en_tete(list); !hors_liste(list) && (b == NULL || x > b[0].x); suivant(list))
    valeur_elt(list, (void **)&b);
}

void MAP_CopyListFromX(t_map *map, t_liste *list, int x_from, int x_to) {
  t_block *b;
  for (MAP_SetEcListe(map->list, x_from); (!hors_liste(map->list) && (b == NULL || x_to > b[0].x)); suivant(map->list)) {
    valeur_elt(map->list, (void **)&b);
    ajout_droit(list, b);
  }
}

void MAP_gen(t_map *map) {
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
}

void MAP_afficher_sdl(t_map *map, SDL_Renderer *renderer, int h_aff, int x_deb, int x_fin) {
  t_liste affichage;
  init_liste(&affichage);
  MAP_CopyListFromX(map, &affichage, x_deb, x_fin);
  AFF_map_sdl(&affichage, renderer, h_aff);
  detruire_liste(&affichage, NULL);
}

void MAP_afficher_term(t_map *map, SDL_Renderer *renderer, int h_min_aff, int h_max_aff) { AFF_map_term(map->list, h_min_aff, h_max_aff); }

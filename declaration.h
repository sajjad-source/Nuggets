

void handle_player_join(GameMap* game_map, addr_t from, char* buf);
void handle_player_move(GameMap* game_map, addr_t from, char* buf);
GameMap* initialize_game(const char* map_filename, int seed);
Empty* find_empty_spaces(char** grid, int size, int* count);
void distribute_gold(GameMap* game_map, int goldTotal, int goldMinNumPiles, int goldMaxNumPiles);
void game_over(GameMap* game_map);
char* serialize_map_with_players(GameMap *gameMap, addr_t from);
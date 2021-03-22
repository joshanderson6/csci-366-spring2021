//
// Created by carson on 5/20/20.
//

#include <stdlib.h>
#include <stdio.h>
#include "game.h"
#include <string.h>

// STEP 9 - Synchronization: the GAME structure will be accessed by both players interacting
// asynchronously with the server.  Therefore the data must be protected to avoid race conditions.
// Add the appropriate synchronization needed to ensure a clean battle.

static game * GAME = NULL;

void game_init() {
    if (GAME) {
        free(GAME);
    }
    GAME = malloc(sizeof(game));
    GAME->status = CREATED;
    game_init_player_info(&GAME->players[0]);
    game_init_player_info(&GAME->players[1]);
}

void game_init_player_info(player_info *player_info) {
    player_info->ships = 0;
    player_info->hits = 0;
    player_info->shots = 0;
}

int game_fire(game *game, int player, int x, int y) {
    // Step 5 - This is the crux of the game.  You are going to take a shot from the given player and
    // update all the bit values that store our game state.
    //
    //  - You will need up update the players 'shots' value
    //  - you You will need to see if the shot hits a ship in the opponents ships value.  If so, record a hit in the
    //    current players hits field
    //  - If the shot was a hit, you need to flip the ships value to 0 at that position for the opponents ships field
    //
    //  If the opponents ships value is 0, they have no remaining ships, and you should set the game state to
    //  PLAYER_1_WINS or PLAYER_2_WINS depending on who won.
}

unsigned long long int xy_to_bitval(int x, int y) {
    // Step 1 - implement this function.  We are taking an x, y position
    // and using bitwise operators, converting that to an unsigned long long
    // with a 1 in the position corresponding to that x, y
    //
    // x:0, y:0 == 0b00000...0001 (the one is in the first position)
    // x:1, y: 0 == 0b00000...10 (the one is in the second position)
    // ....
    // x:0, y: 1 == 0b100000000 (the one is in the eighth position)
    //
    // you will need to use bitwise operators and some math to produce the right
    // value.
    unsigned long long int_64 = 1ull;
    if (x < 8 && x >= 0 && y < 8 && y >= 0) {
        unsigned int move = x + 8 * y;
        int_64 = (int_64 << move);
    }
    else {
        int_64 = 0;
    }
    return int_64;
}

struct game * game_get_current() {
    return GAME;
}

int game_load_board(struct game *game, int player, char * spec) {
    // Step 2 - implement this function.  Here you are taking a C
    // string that represents a layout of ships, then testing
    // to see if it is a valid layout (no off-the-board positions
    // and no overlapping ships)
    //
    // if it is valid, you should write the corresponding unsigned
    // long long value into the Game->players[player].ships data
    // slot and return 1
    //
    // if it is invalid, you should return -1
    int carrier = 5;
    int battleship = 4;
    int destroyer = 3;
    int submarine = 3;
    int patrolBoat = 2;

    int carrier_used = 0;
    int battleship_used = 0;
    int destroyer_used = 0;
    int submarine_used = 0;
    int patrolBoat_used = 0;

    int return_value = -1;

    if (spec == NULL) {
        return return_value;
    }

    int length = strlen(spec);

    if (length != 15) {
        return return_value;
    }

    for (int i = 0; i < length; i += 3) {
        char ship_type = spec[i];
        int x = spec[i + 1] - '0';
        int y = spec[i + 2] - '0';

        if (ship_type == 'C' && carrier_used == 0 && x < 4) {
            carrier_used = 1;
            return_value = add_ship_horizontal(&game->players[player], x, y, carrier);
        }
        else if (ship_type == 'c' && carrier_used == 0 && y < 4) {
            carrier_used = 1;
            return_value = add_ship_vertical(&game->players[player], x, y, carrier);
        }
        else if (ship_type == 'B' && battleship_used == 0 && x < 5) {
            battleship_used = 1;
            return_value = add_ship_horizontal(&game->players[player], x, y, battleship);
        }
        else if (ship_type == 'b' && battleship_used == 0 && y < 5) {
            battleship_used = 1;
            return_value = add_ship_vertical(&game->players[player], x, y, battleship);
        }
        else if (ship_type == 'D' && destroyer_used == 0 && x < 6) {
            destroyer_used = 1;
            return_value = add_ship_horizontal(&game->players[player], x, y, destroyer);
        }
        else if (ship_type == 'd' && destroyer_used == 0 && y < 6) {
            destroyer_used = 1;
            return_value = add_ship_vertical(&game->players[player], x, y, destroyer);
        }
        else if (ship_type == 'S' && submarine_used == 0 && x < 6) {
            submarine_used = 1;
            return_value = add_ship_horizontal(&game->players[player], x, y, submarine);
        }
        else if (ship_type == 's' && submarine_used == 0 && x < 6) {
            submarine_used = 1;
            return_value = add_ship_vertical(&game->players[player], x, y, submarine);
        }
        else if (ship_type == 'P' && patrolBoat_used == 0 && x < 7) {
            patrolBoat_used = 1;
            return_value = add_ship_horizontal(&game->players[player], x, y, patrolBoat);
        }
        else if (ship_type == 'p' && patrolBoat_used == 0 && y < 7) {
            patrolBoat_used = 1;
            return_value = add_ship_vertical(&game->players[player], x, y, patrolBoat);
        }
        else {
            return_value = -1;
        }

        if (return_value == -1) {
            return return_value;
        }
    }
    return return_value;
}
int add_ship_horizontal(player_info *player, int x, int y, int length) {
    // implement this as part of Step 2
    // returns 1 if the ship can be added, -1 if not
    // hint: this can be defined recursively
    if (length <= 0) {
        return 1;
    }
    if (x < 0 || y < 0 || x >= 8 || y >= 8 ) {
        return -1;
    }
    unsigned long long bitmask = xy_to_bitval(x,y);
    if(player->ships & bitmask) {
        return -1;
    }
    else if (player->ships | bitmask) {
        player->ships += bitmask;
    }
    x++;
    length--;
    add_ship_horizontal(player, x, y, length);
}

int add_ship_vertical(player_info *player, int x, int y, int length) {
    // implement this as part of Step 2
    // returns 1 if the ship can be added, -1 if not
    // hint: this can be defined recursively
    if (length <= 0) {
        return 1;
    }
    if (x < 0 || y < 0 || x >= 8 || y >= 8 ) {
        return -1;
    }
    unsigned long long bitmask = xy_to_bitval(x,y);
    if(player->ships & bitmask) {
        return -1;
    }
    else if (player->ships | bitmask) {
        player->ships += bitmask;
    }
    y++;
    length--;
    add_ship_vertical(player, x, y, length);
}
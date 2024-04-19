#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "monopoly.h" // NEMENIT !!!
#include <time.h>

// TODO vlastne funkcie
int hasMonopoly(const PLAYER *player, COLOR color) {
    int property_count = 0;
    int owned_count = 0;

    for (int i = 0; i < NUM_PROPERTIES; ++i) {
        if (properties[i].color == color) {
            property_count++;
        }
    }

    for (int i = 0; i < player->num_properties; ++i) {
        if (player->owned_properties[i]->color == color) {
            owned_count++;
        }
    }

    return owned_count == property_count;
}


int findPropertySpaceNumber(SPACE game_board[], int num_spaces, PROPERTY *prop) {
    for (int i = 0; i < num_spaces; i++) {
        if (game_board[i].property == prop) {
            return i + 1;
        }
    }
    return -1;
}

int hasProperty(const PLAYER *player, const PROPERTY *property) {
    for (int i = 0; i < player->num_properties; i++) {
        if (player->owned_properties[i] == property) {
            return 1;
        }
    }
    return 0;
}

int checkWinner(PLAYER *players, int total_players, int *bankrupt) {
    int max_cash = -1;
    int winner_index = -1;
    int tie_count = 0;

    for (int i = 0; i < total_players; ++i) {
        if (!bankrupt[i] && players[i].cash > max_cash) {
            max_cash = players[i].cash;
            winner_index = i;
            tie_count = 1;
        } else if (!bankrupt[i] && players[i].cash == max_cash) {
            tie_count++;
        }
    }

    if (tie_count == 1) {
        return winner_index;
    }

    int max_total_value = -1;
    tie_count = 0;
    for (int i = 0; i < total_players; ++i) {
        if (!bankrupt[i] && players[i].cash == max_cash) {
            int total_value = players[i].cash;
            for (int j = 0; j < players[i].num_properties; ++j) {
                total_value += players[i].owned_properties[j]->price;
            }
            if (total_value > max_total_value) {
                max_total_value = total_value;
                winner_index = i;
                tie_count = 1;
            } else if (total_value == max_total_value) {
                tie_count++;
            }
        }
    }

    if (tie_count > 1) {
        return -1;
    }

    return winner_index;
}


// functions for print
void printPlayerStatus(PLAYER *players, int total_players) {
    printf("Players:\n");
    for (int i = 0; i < total_players; i++) {

        printf("%d. S: %d, C: %d, JP: %-1d, IJ: %s\n",
               i + 1,
               players[i].space_number,
               players[i].cash,
               players[i].num_jail_pass,
               players[i].is_in_jail ? "yes" : "no");

        for (int j = 0; j < players[i].num_properties; j++) {
            PROPERTY *prop = players[i].owned_properties[j];
            int space_number = findPropertySpaceNumber(game_board, NUM_SPACES, prop);

            printf("    %-18s %2d %-7s S%d\n", prop->name, prop->price, property_colors[prop->color], space_number);
        }
    }
}

void printWinner(PLAYER *players, int total_players, int *bankrupt) {
    int winner = checkWinner(players, total_players, bankrupt);
    if (winner >= 0) {
        printf("WINNER: P%d\n", players[winner].number);
    } else {
        printf("WINNER: ?\n");
    }
}

void printGameBoard(SPACE game_board[], int num_spaces, PLAYER *players, int total_players) {
    printf("Game Board:\n");
    for (int i = 0; i < num_spaces; i++) {
        SPACE space = game_board[i];
        if (space.type == Property) {
            printf("%2d. %-17s %2d %-10s", i + 1, space.property->name, space.property->price, property_colors[space.property->color]);
        } else {
            printf("%2d. %-17s", i + 1, space_types[space.type]);
        }

        if (space.type == Property) {
            int owned = 0;
            for (int j = 0; j < total_players; j++) {
                for (int k = 0; k < players[j].num_properties; k++) {
                    if (players[j].owned_properties[k] == space.property) {
                        printf(" P%d", players[j].number);

                        int monopol = 1;
                        for (int m = 0; m < num_spaces; m++) {
                            if (game_board[m].type == Property && game_board[m].property->color == space.property->color) {
                                if (!hasProperty(&players[j], game_board[m].property)) {
                                    monopol = 0;
                                    break;
                                }
                            }
                        }
                        printf(" %s", monopol ? "yes" : "no");
                        owned = 1;
                        break;
                    }
                }
                if (owned) break;
            }
        }
        printf("\n");
    }
}

void printGame(int roll, int turn, PLAYER *player) {
    printf("\n");
    printf("R: %d\n", roll);
    printf("Turn: %d\n", turn);
    printf("Player on turn: P%d\n", player->number);
}

void setPlayers(PLAYER *players, int amount_of_players, int cash) {
    for (int i = 0; i < amount_of_players; ++i) {
        players[i].number = i + 1;
        players[i].space_number = 1;
        players[i].cash = cash;
        players[i].num_jail_pass = 0;
        players[i].is_in_jail = 0;
        for (int j = 0; j < NUM_PROPERTIES; ++j) {
            players[i].owned_properties[j] = NULL;
        }
        players[i].num_properties = 0;
    }
}

void checkPlayerPosition(SPACE space, PLAYER *player, PLAYER *players, int total_players, int *is_game_over, int *bankrupt, int roll) {
    int is_owned = 0;

    switch (space.type) {
        case Start:
            break;
        case Property:
            for (int i = 0; i < total_players; ++i) {
                if (hasProperty(&players[i], space.property)) {
                    is_owned = 1;
                    if (&players[i] != player) {
                        int monopol = hasMonopoly(&players[i], space.property->color);
                        int rent = space.property->price;
                        if (monopol) {
                            rent *= 2;
                        }

                        if (player->cash < rent) {
                            *is_game_over = 1;
                            bankrupt[player - players] = 1;
                            return;
                        }

                        player->cash -= rent;
                        players[i].cash += rent;
                    }
                    break;
                }
            }

            if (!is_owned && player->cash >= space.property->price) {
                player->owned_properties[player->num_properties++] = space.property;
                player->cash -= space.property->price;
          //    printf("Player P%d bought %s for %d.\n",
           //           player->number, space.property->name, space.property->price);
            } else if (!is_owned && player->cash < space.property->price) {
                *is_game_over = 1;
                bankrupt[player - players] = 1;
                return;
            }
            break;

        case Jail_pass:
            player->num_jail_pass += 1;
            break;
        case In_jail:
            if (player->is_in_jail) {
                player->cash -= 1;
                if (player->cash < 0) {
                    *is_game_over = 1;
                    bankrupt[player - players] = 1;
                    return;
                }
                player->is_in_jail = 0;
                player->space_number += roll;
                SPACE new_space = game_board[player->space_number - 1];
                checkPlayerPosition(new_space, player, players, total_players, is_game_over, bankrupt, roll);
            }
            break;
        case Free_parking:
            break;
        case Go_to_jail:
            if (player->num_jail_pass > 0) {
                player->num_jail_pass--;
            } else {
                player->space_number = 7;
                player->is_in_jail = 1;
            }
            break;
        default:
            break;
    }
}

void handlePassingStart(int roll, PLAYER *player) {
    player->space_number = (player->space_number + roll) - NUM_SPACES;
    player->cash += 2;
}

void printGameStates(int printing_gameboard, int printing_player_info, int printing_game_state, PLAYER *players, int amount_of_players) {
    if (printing_gameboard) {
        printf("\n");
        printGameBoard(game_board, NUM_SPACES, players, amount_of_players);
    }

    if (printing_player_info) {
        printf("\n");
        printPlayerStatus(players, amount_of_players);
    }

    if (printing_game_state) {
        printf("\n");
        printPlayerStatus(players, amount_of_players);
        printGameBoard(game_board, NUM_SPACES, players, amount_of_players);
        printf("WINNER: -\n");
    }
}

void gameStart(PLAYER *players, int amount_of_players, int printing_gameboard, int printing_player_info, int printing_game_state) {
    int is_game_over = 0;
    int turn = 0;
    int roll = 0;

    int bankrupt[amount_of_players];
    for (int i = 0; i < amount_of_players; i++) {
        bankrupt[i] = 0;
    }

    while (!is_game_over) {

        for (int i = 0; i < amount_of_players; ++i) {
            if (scanf("%d", &roll) != 1) {
                if (getchar() == '.') {
                    is_game_over = 1;
                    printf("\n");
                    printPlayerStatus(players, amount_of_players);
                    printGameBoard(game_board, NUM_SPACES, players, amount_of_players);
                    printWinner(players, amount_of_players, bankrupt);
                    break;
                }
                break;
            }

            if (players[i].is_in_jail == 0) {

                if ((players[i].space_number + roll) > NUM_SPACES) {
                    handlePassingStart(roll, &players[i]);
                } else {
                    players[i].space_number += roll;
                }
            }

            checkPlayerPosition(game_board[players[i].space_number - 1], &players[i], players, amount_of_players, &is_game_over, bankrupt, roll);
            turn++;
            printGame(roll, turn, &players[i]);

            printGameStates(printing_gameboard, printing_player_info, printing_game_state, players, amount_of_players);

            if (is_game_over) {
                printf("\n");
                printPlayerStatus(players, amount_of_players);
                printGameBoard(game_board, NUM_SPACES, players, amount_of_players);
                printWinner(players, amount_of_players, bankrupt);
                break;
            }

        }

        if (is_game_over) {
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    int n = 2;
    int cash = 20;
    int printing_gameboard, printing_game_state, printing_player_info = 0;
    int opt;

    while ((opt = getopt(argc, argv, "n:spg")) != -1) {
        switch (opt) {
            case 'n':
                n = atoi(optarg);
                break;
            case 's':
                printing_gameboard = 1;
                break;
            case 'p':
                printing_player_info = 1;
                break;
            case 'g':
                printing_game_state = 1;
            default:
                break;
        }
    }

    switch (n) {
        case 3:
            cash = 18;
            break;
        case 4:
            cash = 16;
            break;
        default:
            break;
    }

    // naplnenie hráčov
    PLAYER *players = (PLAYER *) malloc(n * sizeof(PLAYER));
    setPlayers(players, n, cash);

    // výpis stavu hry
    printPlayerStatus(players, n);
    printGameBoard(game_board, NUM_SPACES, players, n);
    printf("WINNER: -\n");

    // začiatok hry
    gameStart(players, n, printing_gameboard, printing_player_info, printing_game_state);

    free(players);
    return 0;
}

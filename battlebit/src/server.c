//
// Created by carson on 5/20/20.
//

#include "stdio.h"
#include "stdlib.h"
#include "server.h"
#include "char_buff.h"
#include "game.h"
#include "repl.h"
#include "pthread.h"
#include<string.h>    //strlen
#include<sys/socket.h>
#include<arpa/inet.h>    //inet_addr
#include<unistd.h>    //write

static game_server *SERVER;
pthread_mutex_t lock;

void init_server() {
    if (SERVER == NULL) {
        SERVER = calloc(1, sizeof(struct game_server));
    } else {
        printf("Server already started");
    }
}

int handle_client_connect(int player) {
    // STEP 8 - This is the big one: you will need to re-implement the REPL code from
    // the repl.c file, but with a twist: you need to make sure that a player only
    // fires when the game is initialized and it is there turn.  They can broadcast
    // a message whenever, but they can't just shoot unless it is their turn.
    //
    // The commands will obviously not take a player argument, as with the system level
    // REPL, but they will be similar: load, fire, etc.
    //
    // You must broadcast informative messages after each shot (HIT! or MISS!) and let
    // the player print out their current board state at any time.
    //
    // This function will end up looking a lot like repl_execute_command, except you will
    // be working against network sockets rather than standard out, and you will need
    // to coordinate turns via the game::status field.
    int current_socket = SERVER->player_sockets[player];
    char buffer[2000];
    char * prompt = "battleBit (? for help) > ";
    struct char_buff * input_buffer = cb_create(2000);
    struct char_buff * output_buffer = cb_create(2000);


    cb_append(output_buffer, "Welcome to battleBit server Player ");
    cb_append_int(output_buffer, player);
    cb_append(output_buffer, "\n");
    cb_write(current_socket, output_buffer);
    cb_reset(output_buffer);

    cb_append(output_buffer, prompt);
    cb_write(current_socket, output_buffer);
    cb_reset(output_buffer);

    int read_size;
    while ((read_size = recv(current_socket, buffer, 2000, 0)) > 0) {
        pthread_mutex_unlock(&lock);
        cb_append(input_buffer,buffer);
        char* command = cb_tokenize(input_buffer, " \r\n");
        if (command) {
            if (strcmp(command, "exit") == 0) {
                cb_append(output_buffer, "goodbye!");
                cb_write(current_socket, output_buffer);
                cb_reset(output_buffer);
                exit(EXIT_SUCCESS);
            } else if (strcmp(command, "?") == 0) {
                cb_append(output_buffer, "? - show help\n");
                cb_append(output_buffer, "load <string> - load a ship layout file for the current player\n");
                cb_append(output_buffer, "show - shows the board for the current player\n");
                cb_append(output_buffer, "fire [0-7] [0-7] - fire at the given position\n");
                cb_append(output_buffer, "say <string> - Send the string to all players as part of a chat\n");
                cb_append(output_buffer, "reset - reset the game\n");
                cb_append(output_buffer, "server - start the server\n");
                cb_append(output_buffer, "exit - quit the server\n");
                cb_write(current_socket, output_buffer);
                cb_reset(output_buffer);
            } else if (strcmp(command, "server") == 0) {
                server_start();
            } else if (strcmp(command, "show") == 0) {
                struct char_buff *boardBuffer = cb_create(2000);
                repl_print_board(game_get_current(), player, boardBuffer);
                cb_write(current_socket, boardBuffer);
            } else if (strcmp(command, "reset") == 0) {
                game_init();
            } else if (strcmp(command, "load") == 0) {
                char *arg1 = cb_next_token(input_buffer);
                game_load_board(game_get_current(), player, arg1);
                if (game_get_current()->status == CREATED) {
                    cb_append(output_buffer, "Waiting on Player 1\n");
                    server_broadcast(output_buffer);
                    cb_reset(output_buffer);
                } else if (game_get_current()->status == PLAYER_0_TURN) {
                    cb_append(output_buffer, "All Player Boards Loaded\n");
                    cb_append(output_buffer, "Player 0 Turn\n");
                    server_broadcast(output_buffer);
                    cb_reset(output_buffer);
                    cb_append(output_buffer, prompt);
                    cb_write(SERVER->player_sockets[1 - player], output_buffer);
                    cb_reset(output_buffer);
                }
            } else if (strcmp(command, "fire") == 0) {
                if (game_get_current()->status == CREATED) {
                    cb_append(output_buffer, "Game Has Not Begun!\n");
                    cb_write(current_socket, output_buffer);
                    cb_reset(output_buffer);
                } else if (game_get_current()->status == PLAYER_0_TURN && player == 1) {
                    cb_append(output_buffer, "Player 0 Turn\n");
                    cb_write(current_socket, output_buffer);
                    cb_reset(output_buffer);
                } else if (game_get_current()->status == PLAYER_1_TURN && player == 0) {
                    cb_append(output_buffer, "Player 1 Turn\n");
                    cb_write(current_socket, output_buffer);
                    cb_reset(output_buffer);
                } else {
                    char *arg1 = cb_next_token(input_buffer);
                    char *arg2 = cb_next_token(input_buffer);
                    int x = atoi(arg1);
                    int y = atoi(arg2);
                    if (x < 0 || x >= BOARD_DIMENSION || y < 0 || y >= BOARD_DIMENSION) {
                        cb_append(output_buffer, "Invalid coordinate: ");
                        cb_append_int(output_buffer, x);
                        cb_append(output_buffer, " ");
                        cb_append_int(output_buffer, y);
                        cb_append(output_buffer, "\n");
                        cb_write(current_socket, output_buffer);
                        cb_reset(output_buffer);
                    } else {
                        cb_append(output_buffer, "Player ");
                        cb_append_int(output_buffer, player);
                        cb_append(output_buffer, " fired at ");
                        cb_append_int(output_buffer, x);
                        cb_append(output_buffer, " ");
                        cb_append_int(output_buffer, y);

                        int result = game_fire(game_get_current(), player, x, y);
                        if (result) {
                            cb_append(output_buffer, "  HIT");
                            if (game_get_current()->players[1 - player].ships == 0) {
                                cb_append(output_buffer, " PLAYER ");
                                cb_append_int(output_buffer, player);
                                cb_append(output_buffer, " WINS!\n");
                                server_broadcast(output_buffer);
                                cb_reset(output_buffer);
                                exit(EXIT_SUCCESS);
                            }
                            cb_append(output_buffer, "\n");
                        } else {
                            cb_append(output_buffer, "  Miss\n");
                        }
                    }
                    server_broadcast(output_buffer);
                    cb_reset(output_buffer);
                    cb_append(output_buffer, prompt);
                    cb_write(SERVER->player_sockets[1 - player], output_buffer);
                    cb_reset(output_buffer);

                }
            } else if (strcmp(command, "say") == 0) {
                char *arg1 = strtok(input_buffer->tokenization_save_pointer, "\r");
                cb_append(output_buffer, "\nPlayer ");
                cb_append_int(output_buffer, player);
                cb_append(output_buffer, " says: ");
                cb_append(output_buffer, arg1);
                cb_append(output_buffer, "\n");
                cb_write(SERVER->player_sockets[1 - player], output_buffer);
                cb_reset(output_buffer);
                cb_append(output_buffer, prompt);
                cb_write(SERVER->player_sockets[1 - player], output_buffer);
                cb_reset(output_buffer);

            } else {
                cb_append(output_buffer, "Unknown Command: ");
                cb_append(output_buffer, command);
                cb_append(output_buffer, "\n");
                cb_write(current_socket, output_buffer);
                cb_reset(output_buffer);
            }

            cb_append(output_buffer, prompt);
            cb_write(current_socket, output_buffer);
            cb_reset(output_buffer);
            cb_reset(input_buffer);
        }
        pthread_mutex_lock(&lock);
    }
    return 0;
}

void server_broadcast(char_buff *msg) {
    // send message to all players
    cb_write(SERVER->player_sockets[0], msg);
    cb_write(SERVER->player_sockets[1], msg);
    printf("%s",msg->buffer);
}

int run_server() {
    // STEP 7 - implement the server code to put this on the network.
    // Here you will need to initalize a server socket and wait for incoming connections.
    //
    // When a connection occurs, store the corresponding new client socket in the SERVER.player_sockets array
    // as the corresponding player position.
    //
    // You will then create a thread running handle_client_connect, passing the player number out
    // so they can interact with the server asynchronously

    int server_socket_fd = socket(AF_INET,
                                  SOCK_STREAM,
                                  IPPROTO_TCP);
    if (server_socket_fd == -1) {
        printf("Could not create socket\n");
    }

    int yes = 1;
    setsockopt(server_socket_fd,
               SOL_SOCKET,
               SO_REUSEADDR, &yes, sizeof(yes));

    struct sockaddr_in server;

    // fill out the socket information
    server.sin_family = AF_INET;
    // bind the socket on all available interfaces
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(BATTLEBIT_PORT);

    int request = 0;
    if (bind(server_socket_fd,
            // Again with the cast
             (struct sockaddr *) &server,
             sizeof(server)) < 0) {
        puts("Bind failed");
    } else {
        puts("Bind worked!");
        listen(server_socket_fd, BATTLEBIT_PORT);

        //Accept an incoming connection
        puts("Waiting for incoming connections...");

        struct sockaddr_in client;
        socklen_t size_from_connect;
        int client_socket_fd;
        int player = 0;
        pthread_mutex_init(&lock, NULL);
        while ((client_socket_fd = accept(server_socket_fd, (struct sockaddr *) &client, &size_from_connect)) > 0) {
            SERVER->player_sockets[player] = client_socket_fd;
            pthread_create(&SERVER->player_threads[player], NULL, (void *) handle_client_connect, player);
            player++;
        }
    }
}

int server_start() {
    // STEP 6 - using a pthread, run the run_server() function asynchronously, so you can still
    // interact with the game via the command line REPL
    init_server();
    pthread_create(&SERVER->server_thread, NULL, (void *) run_server, NULL);
}

/**
 *  This is example of game, that you can use for debugging.
 */
#include <cstdlib>
#include <ctime>
#include <iostream>

#include "my_player.h"
#include "silly_player.hpp"
#include "game_engine.h"

int main() {
    srand(time(NULL));

    /// Creates observer to log every event to std::cout
    BasicObserver obs(std::cout);

    /// Create setting for game
    GameSettings settings = {
        .field_size = {
            .min = {.x = -7, .y = -7},
            .max = {.x = 7, .y = 7},
        },
        .max_moves = 0,
        .win_length = 5,
        .move_timelimit_ms = 0,
    };

    /// Creates game engine with given settings
    GameEngine game(settings);

    /// Adds observer to the view of this game engine
    game.get_view().add_observer(obs);
    /// Creates first player
    myPlayer player1("Vasya");
    /// Adds it to play Xs
    game.set_player(player1);
    /// Create second player
    SimplePlayer player2("Kolya");
    /// Adds it to play Os
    game.set_player(player2);
    /// Starts game until someone wins.
    game.play_game();


}

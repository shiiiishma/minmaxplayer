#pragma once
#include <ostream>
#include "player.h"
#include <vector>
#include "game_engine.h"



/**
 * This is example player, that plays absolutely randomly.
 */
class RandomPlayer : public Player {
    std::string _name;
public:
    RandomPlayer(const std::string& name) : _name(name) {}
    std::string get_name() const override { return _name; }
    Point play(const GameView& game) override;
    void assign_mark(Mark player_mark) override { /*does nothing*/ }
    void notify(const GameView&, const Event&) override { /*does nothing*/ }
};


/**
 * Simple observer, that logs every event in given output stream.
 */
class BasicObserver : public Observer {
    std::ostream& _out;

    std::ostream& _print_mark(Mark mark);
public:
    BasicObserver(std::ostream& out_stream) : _out(out_stream) {}
    void notify(const GameView&, const Event& event) override;
};

class FieldCache {
    Boundary cache_size;
    std::vector<bool> _filled;
    std::vector<bool> _crosses;


public:
    size_t _to_index(const Point& p) const;
    Point _to_point(size_t i) const;
    Mark _get_value(size_t i) const;
    FieldCache(const Boundary& field_size);
    bool is_winning_move(const Point& p) const;
    void set(const Point& p, Mark m);
    void reset(const Point& p);
    Mark get_winning_player();
};

class myPlayer : public Player {
    std::string _name;
    Point last_move;
    Mark my_mark;
public:
    myPlayer(const std::string& name) : _name(name) {}
    std::string get_name() const override { return _name; }
    Point play(const GameView& game) override;
    void assign_mark(Mark player_mark) override;
    void notify(const GameView&, const Event&) override;
    Point BestCoords(const GameView& game);
    int minmax(const GameView& game, FieldCache& field, int dx, int dy, int number_move, int glubina);
    bool is_finished(const GameView& game, int** field);
};



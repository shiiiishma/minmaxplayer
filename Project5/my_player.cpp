#include "my_player.h"
#include <cstdlib>
#include <iostream>
#include <fstream>

static field_index_t rand_int(field_index_t min, field_index_t max) {
    return min + rand() % (max - min + 1);
}

Point RandomPlayer::play(const GameView& game) {
    Boundary b = game.get_settings().field_size;
    Point result;
    do {
        result = {
            .x = rand_int(b.min.x, b.max.x),
            .y = rand_int(b.min.y, b.max.y),
        };
    } while (game.get_state().field->get_value(result) != Mark::None);
    return result;
}

void BasicObserver::notify(const GameView&, const Event& event) {
    if (event.get_type() == MoveEvent::TYPE) {
        auto& data = get_data<MoveEvent>(event);
        _out << "Move:\tx = " << data.point.x
            << ",\ty = " << data.point.y << ":\t";
        _print_mark(data.mark) << '\n';
        return;
    }


    if (event.get_type() == PlayerJoinedEvent::TYPE) {
        auto& data = get_data<PlayerJoinedEvent>(event);
        _out << "Player '" << data.name << "' joined as ";
        _print_mark(data.mark) << '\n';
        return;
    }
    if (event.get_type() == GameStartedEvent::TYPE) {
        _out << "Game started\n";
        return;
    }


    if (event.get_type() == WinEvent::TYPE) {
        auto& data = get_data<WinEvent>(event);
        _out << "Player playing ";
        _print_mark(data.winner_mark) << " has won\n";
        return;
    }

    if (event.get_type() == DrawEvent::TYPE) {
        auto& data = get_data<DrawEvent>(event);
        _out << "Draw happened, reason: '" << data.reason << "'\n";
        return;
    }

    if (event.get_type() == DisqualificationEvent::TYPE) {
        auto& data = get_data<DisqualificationEvent>(event);
        _out << "Player playing ";
        _print_mark(data.player_mark) << " was disqualified, reason: '"
            << data.reason << "'\n";
        return;
    }
}

std::ostream& BasicObserver::_print_mark(Mark m) {
    if (m == Mark::Cross) return _out << "X";
    if (m == Mark::Zero) return _out << "O";
    return _out << "?";
}


Point myPlayer::BestCoords(const GameView& game) {
    int max = -100000;
    int max_y, max_x;
    Boundary b = game.get_settings().field_size;
    int move_number = game.get_state().number_of_moves;
    FieldCache field(b);
    for (auto it = game.get_state().field->get_iterator(); it->has_value(); it->step()) {
        field.set(it->get_point(), it->get_value());
    }

    for (int dx = b.min.x; dx < b.max.x + 1; dx++) {
        for (int dy = b.min.y; dy < b.max.y + 1; dy++) {
            Point p{ dx, dy };
            if (field._get_value(field._to_index(p)) == Mark::None) {
                int res = minmax(game, field, dx, dy, move_number, 0);
                if (max < res) {
                    max = res;
                    max_x = dx;
                    max_y = dy;
                }
                if (res == 100) {
                    return Point{ dx, dy };
                }
            }
        }
    }






    return Point{ max_x, max_y };

}

int myPlayer::minmax(const GameView& game, FieldCache& field, int dx, int dy, int move_number, int glubina) {
    Boundary b = game.get_settings().field_size;
    int res = 0;
    if (move_number == (b.max.x + 1) * (b.max.y + 1) || glubina > 8) {
        return 0;
    }
    Point p{ dx, dy };
    move_number++;
    field.set(p, (move_number % 2) == 1 ? Mark::Cross : Mark::Zero);

    if (my_mark == Mark::Cross) {
        if (field.is_winning_move(p) && (move_number % 2) == 1) {
            field.reset(p);
            return 100;
        }
        else if (field.is_winning_move(p) && (move_number % 2) == 0) {
            field.reset(p);
            return -100;
        }
        else {
            for (int dx = b.min.x; dx < b.max.x + 1; dx++) {
                for (int dy = b.min.x; dy < b.max.y + 1; dy++) {
                    Point coord{ dx, dy };
                    if (field._get_value(field._to_index(coord)) == Mark::None) {
                        res += minmax(game, field, dx, dy, move_number, ++glubina);
                    }
                }
            }



        }
    }
    else {
        if (field.is_winning_move(p) && (move_number % 2) == 0) {
            field.reset(p);
            return 100;
        }
        else if (field.is_winning_move(p) && (move_number % 2) == 1) {
            field.reset(p);
            return -100;
        }
        else {
            for (int dx = b.min.x; dx < b.max.x + 1; dx++) {
                for (int dy = b.min.x; dy < b.max.y + 1; dy++) {
                    Point coord{ dx, dy };
                    if (field._get_value(field._to_index(coord)) == Mark::None) {
                        res += minmax(game, field, dx, dy, move_number, ++glubina);
                    }
                }
            }

        }
    }
    field.reset(p);
    return res;

}


size_t FieldCache::_to_index(const Point& p) const
{
    const size_t i = p.y - cache_size.min.y, j = p.x - cache_size.min.x;
    return i * cache_size.get_width() + j;
}

Point FieldCache::_to_point(size_t i) const
{
    const size_t width = cache_size.get_width();
    return {
        .x = static_cast<field_index_t>(cache_size.min.x + i % width),
        .y = static_cast<field_index_t>(cache_size.min.y + i / width),
    };
}

Mark FieldCache::_get_value(size_t i) const
{
    return _filled.at(i)
        ? (_crosses.at(i) ? Mark::Cross : Mark::Zero)
        : Mark::None;
}

FieldCache::FieldCache(const Boundary& field_size) : cache_size(field_size), _filled(), _crosses() {
    size_t n_cells = field_size.get_width() * field_size.get_height();
    _filled.resize(n_cells, false);
    _crosses.resize(n_cells, false);
}

void FieldCache::set(const Point& p, Mark m)
{
    const size_t i = _to_index(p);
    switch (m) {
    case Mark::Cross:
        _filled.at(i) = true;
        _crosses.at(i) = true;
        break;
    case Mark::Zero:
        _filled.at(i) = true;
        _crosses.at(i) = false;
        break;
    case Mark::None:
        _filled.at(i) = false;
        break;
    }
}

void FieldCache::reset(const Point& p)
{
    set(p, Mark::None);
}


void myPlayer::assign_mark(Mark player_mark) {
    my_mark = player_mark;
}

bool FieldCache::is_winning_move(const Point& move) const {
    const Mark mark = _get_value(_to_index(move));
    for (int dx = 0; dx <= 1; ++dx) {
        for (int dy = -1; dy <= 1; ++dy) {
            if (dx == 0 && (dy == 0 || dy == -1)) continue;
            int max_forward = 0, max_backward = 0;
            Point p;

            p = move;
            do {
                p.x += dx;
                p.y += dy;
                ++max_forward;
            } while (max_forward <= 5
                && cache_size.is_within(p)
                && _get_value(_to_index(p)) == mark);

            p = move;
            do {
                p.x -= dx;
                p.y -= dy;
                ++max_backward;
            } while (max_backward <= 5 - max_forward + 1
                && cache_size.is_within(p)
                && _get_value(_to_index(p)) == mark);

            if (max_forward + max_backward - 1 >= 5) {
                return true;
            }
        }
    }
    return false;
}

void myPlayer::notify(const GameView& view, const Event& event) {
    if (event.get_type() == MoveEvent::TYPE) {
        auto& data = get_data<MoveEvent>(event);
        if (data.mark != my_mark) {
            last_move = data.point;
        }
        return;
    }
}
#include "Move.h"

Move::Move(){};

Move::Move(int col, int row) {
    this->row = row;
    this->col = col;
}

bool Move::operator==(const Move &other) const {
    return this->row == other.row and this->col == other.col;
}

bool Move::operator!=(const Move &other) const { return !(this == &other); }

std::ostream &operator<<(std::ostream &strm, const Move &move) {
    return strm << "Move(" << move.col << ", " << move.row << ")";
}

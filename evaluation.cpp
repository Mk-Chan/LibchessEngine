#include "evaluation.h"

using namespace libchess;

namespace eval {

int tapered_score(std::array<int, 2> score, int phase) {
    return ((score[MIDGAME] * phase) + (score[ENDGAME] * (MAX_PHASE - phase))) / MAX_PHASE;
}

int evaluate(const Position& pos) {
    std::array<int, 2> score{0, 0};

    int phase = 0;
    for (auto& color : constants::COLORS) {
        for (auto& piece_type : constants::PIECE_TYPES) {
            Bitboard piece_bb = pos.piece_type_bb(piece_type, color);
            int num_piece = piece_bb.popcount();

            // Material
            score[MIDGAME] += num_piece * MATERIAL[piece_type][MIDGAME];
            score[ENDGAME] += num_piece * MATERIAL[piece_type][ENDGAME];

            // Piece Square Tables
            while (piece_bb) {
                Square sq = piece_bb.forward_bitscan();
                piece_bb.forward_popbit();
                score[MIDGAME] += PSQT[color][piece_type][sq][MIDGAME];
                score[ENDGAME] += PSQT[color][piece_type][sq][ENDGAME];
            }

            phase += num_piece * PIECE_PHASE[piece_type];
        }
        score[MIDGAME] = -score[MIDGAME];
        score[ENDGAME] = -score[ENDGAME];
    }

    int eval = tapered_score(score, phase);
    if (pos.side_to_move() == constants::BLACK) {
        eval = -eval;
    }
    return eval;
}

} // namespace eval

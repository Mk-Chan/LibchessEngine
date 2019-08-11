#include "evaluation.h"

using namespace libchess;

namespace eval {

int tapered_score(std::array<int, 2> score, int phase) {
    return ((score[MIDGAME] * phase) + (score[ENDGAME] * (MAX_PHASE - phase))) / MAX_PHASE;
}

int evaluate(const Position& pos) {
    std::array<int, 2> score{0, 0};

    Bitboard pawn_bb = pos.piece_type_bb(constants::PAWN);

    int phase = 0;
    for (auto& color : constants::COLORS) {
        for (auto& piece_type : constants::PIECE_TYPES) {
            Bitboard piece_bb = pos.piece_type_bb(piece_type, color);
            int num_piece = piece_bb.popcount();

            // Phase
            phase += num_piece * PIECE_PHASE[piece_type];

            // Material
            score[MIDGAME] += num_piece * MATERIAL[piece_type][MIDGAME];
            score[ENDGAME] += num_piece * MATERIAL[piece_type][ENDGAME];

            // Piece Square Tables
            Bitboard bb = piece_bb;
            while (bb) {
                Square sq = bb.forward_bitscan();
                bb.forward_popbit();
                score[MIDGAME] += PSQT[color][piece_type][sq][MIDGAME];
                score[ENDGAME] += PSQT[color][piece_type][sq][ENDGAME];
            }

            // Pawn eval
            if (piece_type == constants::PAWN) {
                bb = piece_bb;
                while (bb) {
                    Square sq = bb.forward_bitscan();
                    bb.forward_popbit();

                    if (lookups::north(sq) & (pawn_bb & pos.color_bb(color))) {
                        score[MIDGAME] += DOUBLED_PAWNS_MG;
                        score[ENDGAME] += DOUBLED_PAWNS_EG;
                    }

                    Bitboard isolated_pawn_mask = [&]() {
                        Bitboard bb;
                        File sq_file = sq.file();
                        if (sq_file != constants::FILE_H) {
                            bb |= lookups::file_mask(File{sq_file + 1});
                        }
                        if (sq_file != constants::FILE_A) {
                            bb |= lookups::file_mask(File{sq_file - 1});
                        }
                        return bb;
                    }();
                    if (!(isolated_pawn_mask & (pawn_bb & pos.color_bb(color)))) {
                        score[MIDGAME] += ISOLATED_PAWNS_MG;
                        score[ENDGAME] += ISOLATED_PAWNS_EG;
                    }
                }
            }

            // Rook eval
            if (piece_type == constants::ROOK) {
                // Rook on 7th rank
                Bitboard rook_7th_rank_bb =
                    (piece_bb & lookups::relative_rank_mask(constants::RANK_7, color));
                score[MIDGAME] += rook_7th_rank_bb.popcount() * ROOK_7TH_RANK_MG;
                score[ENDGAME] += rook_7th_rank_bb.popcount() * ROOK_7TH_RANK_EG;
            }
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

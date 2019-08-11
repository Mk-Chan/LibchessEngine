#ifndef EVALUATION_H
#define EVALUATION_H

#include "libchess/Position.h"

#include <array>

namespace eval {

enum Stage : int { MIDGAME, ENDGAME };

inline const int MAX_PHASE = 256;

inline const std::array<int, 6> PIECE_PHASE{{1, 10, 10, 20, 40, 0}};

inline std::array<std::array<int, 2>, 6> MATERIAL{{
    {147, 116},
    {378, 518},
    {414, 547},
    {741, 658},
    {1335, 1474},
    {0, 0},
}};

inline int ROOK_7TH_RANK_MG = 101;
inline int ROOK_7TH_RANK_EG = -3;

inline int DOUBLED_PAWNS_MG = -13;
inline int DOUBLED_PAWNS_EG = -43;

inline int ISOLATED_PAWNS_MG = -31;
inline int ISOLATED_PAWNS_EG = -5;

// clang-format off
inline std::array<std::array<std::array<int, 2>, 32>, 6> PSQT_TMP = {
    {{{	// Pawn
          {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0},
          {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0},
          {  0,   5}, {  0,   5}, {  0,   5}, { 10,   5},
          {  0,  10}, {  0,  10}, {  0,  10}, { 20,  10},
          {  0,  30}, {  0,  30}, {  0,  30}, { 15,  30},
          {  0,  50}, {  0,  50}, {  0,  50}, {  0,  50},
          {  0,  80}, {  0,  80}, {  0,  80}, {  0,  80},
          {  0,   0}, {  0,   0}, {  0,   0}, {  0,   0}
      }},
     {{	// Knight
          {-50, -30}, {-30, -20}, {-20, -10}, {-15,   0},
          {-30, -20}, { -5, -10}, {  0,  -5}, {  5,   5},
          {-10, -10}, {  0,  -5}, {  5,   5}, { 10,  10},
          {-10,   0}, {  5,   0}, { 10,  15}, { 20,  20},
          {-10,   0}, {  5,   0}, { 10,  15}, { 20,  20},
          {-10, -10}, {  0,  -5}, {  5,   5}, { 10,  10},
          {-30, -20}, { -5, -10}, {  0,  -5}, {  5,   5},
          {-50, -30}, {-30, -20}, {-20, -10}, {-15,   0}
      }},
     {{	// Bishop
          {-20, -20}, {-20, -15}, {-20, -10}, {-20, -10},
          {-10,   0}, {  0,   0}, { -5,   0}, {  0,   0},
          { -5,   0}, {  5,   0}, {  5,   0}, {  5,   0},
          {  0,   0}, {  5,   0}, { 10,   0}, { 15,   0},
          {  0,   0}, {  5,   0}, { 10,   0}, { 15,   0},
          { -5,   0}, {  5,   0}, {  5,   0}, {  5,   0},
          {-10,   0}, {  0,   0}, { -5,   0}, {  0,   0},
          {-20, -20}, {-20, -15}, {-20, -10}, {-20, -10}
      }},
     {{	// Rook
          { -5,  -5}, {  0,  -3}, {  2,  -1}, {  5,   0},
          { -5,   0}, {  0,   0}, {  2,   0}, {  5,   0},
          { -5,   0}, {  0,   0}, {  2,   0}, {  5,   0},
          { -5,   0}, {  0,   0}, {  2,   0}, {  5,   0},
          { -5,   0}, {  0,   0}, {  2,   0}, {  5,   0},
          { -5,   0}, {  0,   0}, {  2,   0}, {  5,   0},
          { -5,   0}, {  0,   0}, {  2,   0}, {  5,   0},
          { -5,  -5}, {  0,  -3}, {  2,  -1}, {  5,   0}
      }},
     {{
          // Queen
          {-10, -20}, { -5, -10}, { -5,  -5}, { -5,   0},
          { -5, -10}, {  0,  -5}, {  0,   0}, {  0,   5},
          { -5,  -5}, {  0,   5}, {  0,   5}, {  0,  10},
          { -5,   0}, {  0,   5}, {  0,  10}, {  0,  15},
          { -5,   0}, {  0,   5}, {  0,  10}, {  0,  15},
          { -5,  -5}, {  0,   5}, {  0,   5}, {  0,  10},
          { -5, -10}, {  0,  -5}, {  0,   0}, {  0,   5},
          {-10, -20}, { -5, -10}, { -5,  -5}, { -5,   0}
      }},
     {{
          // King
          { 30, -70}, { 45, -45}, { 10, -35}, {-10, -20},
          { 10, -40}, { 20, -25}, {  0, -10}, {-15,   5},
          {-20, -30}, {-25, -15}, {-30,   5}, {-30,  10},
          {-40, -20}, {-50,   5}, {-60,  10}, {-70,  20},
          {-70, -20}, {-80,   5}, {-90,  10}, {-90,  20},
          {-70, -30}, {-80, -15}, {-90,   5}, {-90,  10},
          {-80, -40}, {-80, -25}, {-90, -10}, {-90,   0},
          {-90, -70}, {-90, -45}, {-90, -15}, {-90, -20}
      }}
    }};
// clang-format on

inline const std::array<std::array<std::array<std::array<int, 2>, 64>, 6>, 2> PSQT = []() {
    std::array<std::array<std::array<std::array<int, 2>, 64>, 6>, 2> psqt{};
    for (int c = 0; c < 2; ++c) {
        int k = 0;
        for (int rank = 0; rank < 8; ++rank) {
            for (int file = 0; file < 4; ++file) {
                int sq1 = (rank << 3) | file;
                int sq2 = (rank << 3) | (7 - file);
                if (c == 1) {
                    sq1 ^= 56;
                    sq2 ^= 56;
                }
                for (int pt = 0; pt < 6; ++pt) {
                    psqt[c][pt][sq1] = psqt[c][pt][sq2] = PSQT_TMP[pt][k];
                    psqt[c][pt][sq1] = psqt[c][pt][sq2] = PSQT_TMP[pt][k];
                }
                ++k;
            }
        }
    }
    return psqt;
}();

int evaluate(const libchess::Position&);

} // namespace eval

#endif // EVALUATION_H

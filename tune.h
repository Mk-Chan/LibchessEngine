#ifndef LIBCHESSENGINE__TUNE_H
#define LIBCHESSENGINE__TUNE_H

#include <iomanip>
#include <iostream>
#include <sstream>

#include "evaluation.h"
#include "libchess/Position.h"
#include "libchess/Tuner.h"

inline void tune_handler(std::istringstream& line_stream) {
    std::string line;
    line_stream >> std::quoted(line);
    auto normalized_results = libchess::NormalizedResult<libchess::Position>::parse_epd(
        line, [](const std::string& fen) { return *libchess::Position::from_fen(fen); });
    std::vector<libchess::TunableParameter> tunable_params{{
        {"PawnMG", eval::MATERIAL[libchess::constants::PAWN][eval::MIDGAME]},
        {"PawnEG", eval::MATERIAL[libchess::constants::PAWN][eval::ENDGAME]},
        {"KnightMG", eval::MATERIAL[libchess::constants::KNIGHT][eval::MIDGAME]},
        {"KnightEG", eval::MATERIAL[libchess::constants::KNIGHT][eval::ENDGAME]},
        {"BishopMG", eval::MATERIAL[libchess::constants::BISHOP][eval::MIDGAME]},
        {"BishopEG", eval::MATERIAL[libchess::constants::BISHOP][eval::ENDGAME]},
        {"RookMG", eval::MATERIAL[libchess::constants::ROOK][eval::MIDGAME]},
        {"RookEG", eval::MATERIAL[libchess::constants::ROOK][eval::ENDGAME]},
        {"QueenMG", eval::MATERIAL[libchess::constants::QUEEN][eval::MIDGAME]},
        {"QueenEG", eval::MATERIAL[libchess::constants::QUEEN][eval::ENDGAME]},
        {"Rook7thRankMG", eval::ROOK_7TH_RANK_MG},
        {"Rook7thRankEG", eval::ROOK_7TH_RANK_EG},
        {"DoubledPawnMG", eval::DOUBLED_PAWNS_MG},
        {"DoubledPawnEG", eval::DOUBLED_PAWNS_EG},
        {"IsolatedPawnMG", eval::ISOLATED_PAWNS_MG},
        {"IsolatedPawnEG", eval::ISOLATED_PAWNS_EG},
    }};
    std::cout << "tuning..."
              << "\n";
    auto eval_function = [](libchess::Position& pos,
                            const std::vector<libchess::TunableParameter>& params) -> int {
        for (auto& param : params) {
            if (param.name() == "PawnMG") {
                eval::MATERIAL[libchess::constants::PAWN] = {
                    eval::MATERIAL[libchess::constants::PAWN][eval::MIDGAME],
                    param.value(),
                };
            } else if (param.name() == "PawnEG") {
                eval::MATERIAL[libchess::constants::PAWN] = {
                    param.value(),
                    eval::MATERIAL[libchess::constants::PAWN][eval::ENDGAME],
                };
            } else if (param.name() == "KnightMG") {
                eval::MATERIAL[libchess::constants::KNIGHT] = {
                    eval::MATERIAL[libchess::constants::KNIGHT][eval::MIDGAME],
                    param.value(),
                };
            } else if (param.name() == "KnightEG") {
                eval::MATERIAL[libchess::constants::KNIGHT] = {
                    param.value(),
                    eval::MATERIAL[libchess::constants::KNIGHT][eval::ENDGAME],
                };
            } else if (param.name() == "BishopMG") {
                eval::MATERIAL[libchess::constants::BISHOP] = {
                    eval::MATERIAL[libchess::constants::BISHOP][eval::MIDGAME],
                    param.value(),
                };
            } else if (param.name() == "BishopEG") {
                eval::MATERIAL[libchess::constants::BISHOP] = {
                    param.value(),
                    eval::MATERIAL[libchess::constants::BISHOP][eval::ENDGAME],
                };
            } else if (param.name() == "RookMG") {
                eval::MATERIAL[libchess::constants::ROOK] = {
                    eval::MATERIAL[libchess::constants::ROOK][eval::MIDGAME],
                    param.value(),
                };
            } else if (param.name() == "RookEG") {
                eval::MATERIAL[libchess::constants::ROOK] = {
                    param.value(),
                    eval::MATERIAL[libchess::constants::ROOK][eval::ENDGAME],
                };
            } else if (param.name() == "QueenMG") {
                eval::MATERIAL[libchess::constants::QUEEN] = {
                    eval::MATERIAL[libchess::constants::QUEEN][eval::MIDGAME],
                    param.value(),
                };
            } else if (param.name() == "QueenEG") {
                eval::MATERIAL[libchess::constants::QUEEN] = {
                    param.value(),
                    eval::MATERIAL[libchess::constants::QUEEN][eval::ENDGAME],
                };
            } else if (param.name() == "Rook7thRankMG") {
                eval::ROOK_7TH_RANK_MG = param.value();
            } else if (param.name() == "Rook7thRankEG") {
                eval::ROOK_7TH_RANK_EG = param.value();
            } else if (param.name() == "DoubledPawnMG") {
                eval::DOUBLED_PAWNS_MG = param.value();
            } else if (param.name() == "DoubledPawnEG") {
                eval::DOUBLED_PAWNS_EG = param.value();
            } else if (param.name() == "IsolatedPawnMG") {
                eval::ISOLATED_PAWNS_MG = param.value();
            } else if (param.name() == "IsolatedPawnEG") {
                eval::ISOLATED_PAWNS_EG = param.value();
            }
        }
        int evaluation = eval::evaluate(pos);
        return pos.side_to_move() == libchess::constants::WHITE ? evaluation : -evaluation;
    };
    libchess::Tuner<libchess::Position> tuner{normalized_results, tunable_params, eval_function};
    std::cout << "Initial error: " << tuner.error() << "\n";
    tuner.local_tune();
    tuner.display();
    std::cout << "Done!\n";
}

#endif // LIBCHESSENGINE__TUNE_H

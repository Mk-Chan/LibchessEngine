#include <chrono>

#include "evaluation.h"
#include "search.h"

using namespace libchess;
using namespace eval;

namespace search {

struct SearchStack {
    static std::array<SearchStack, MAX_PLY> new_search_stack() noexcept {
        std::array<SearchStack, MAX_PLY> search_stack{};
        for (unsigned i = 0; i < search_stack.size(); ++i) {
            auto& ss = search_stack[i];
            ss.ply = int(i);
        }
        return search_stack;
    }

    int ply;
};

int qsearch_impl(Position& pos, int alpha, int beta, SearchStack* ss, SearchGlobals& sg) {
    if (sg.stop()) {
        return 0;
    }

    sg.increment_nodes();

    if (ss->ply >= MAX_PLY) {
        return evaluate(pos);
    }

    int eval = evaluate(pos);
    if (eval > alpha) {
        alpha = eval;
    }
    if (eval >= beta) {
        return beta;
    }

    MoveList move_list;
    if (pos.in_check()) {
        move_list = pos.check_evasion_move_list();

        if (move_list.empty()) {
            return pos.in_check() ? -MATE_SCORE + ss->ply : 0;
        }
    } else {
        pos.generate_capture_moves(move_list, pos.side_to_move());
        pos.generate_promotions(move_list, pos.side_to_move());
    }

    int best_score = -INFINITY;
    for (auto move : move_list) {
        if (!pos.is_legal_generated_move(move)) {
            continue;
        }
        pos.make_move(move);
        int score = -qsearch_impl(pos, -beta, -alpha, ss + 1, sg);
        pos.unmake_move();

        if (sg.stop()) {
            return 0;
        }

        if (score > best_score) {
            best_score = score;
        }
        if (best_score >= beta) {
            alpha = beta;
            break;
        }
        if (best_score > alpha) {
            alpha = best_score;
        }
    }

    return alpha;
}

SearchResult search_impl(Position& pos, int alpha, int beta, int depth, SearchStack* ss,
                         SearchGlobals& sg) {
    if (depth <= 0) {
        return {qsearch_impl(pos, alpha, beta, ss, sg), {}};
    }

    if (ss->ply) {
        if (sg.stop()) {
            return {0, {}};
        }

        if (pos.halfmoves() >= 100 || pos.is_repeat()) {
            return {0, {}};
        }

        if (ss->ply >= MAX_PLY) {
            return {evaluate(pos), {}};
        }

        alpha = std::max((-MATE_SCORE + ss->ply), alpha);
        beta = std::max((MATE_SCORE - ss->ply), beta);
        if (alpha >= beta) {
            return {alpha, {}};
        }
    }

    sg.increment_nodes();

    MoveList pv;
    int best_score = -INFINITY;
    auto move_list = pos.legal_move_list();

    if (move_list.empty()) {
        return {pos.in_check() ? -MATE_SCORE + ss->ply : 0, {}};
    }

    for (auto move : move_list) {
        pos.make_move(move);
        SearchResult search_result = -search_impl(pos, -beta, -alpha, depth - 1, ss + 1, sg);
        auto& [score, child_pv] = search_result;
        pos.unmake_move();

        if (ss->ply && sg.stop()) {
            return {0, {}};
        }

        if (score > best_score) {
            best_score = score;
        }
        if (best_score >= beta) {
            alpha = beta;
            break;
        }
        if (best_score > alpha) {
            alpha = score;
            pv.clear();
            pv.add(move);
            if (child_pv) {
                pv.add(*child_pv);
            }
        }
    }

    return {alpha, pv};
}

int qsearch(Position& pos) {
    auto search_stack = SearchStack::new_search_stack();
    auto search_globals = SearchGlobals::new_search_globals();
    return qsearch_impl(pos, -INFINITY, +INFINITY, search_stack.begin(), search_globals);
}

SearchResult search(Position& pos, SearchGlobals& sg, int depth) {
    auto search_stack = SearchStack::new_search_stack();
    int alpha = -INFINITY;
    int beta = +INFINITY;
    SearchResult search_result = search_impl(pos, alpha, beta, depth, search_stack.begin(), sg);
    return search_result;
}

SearchResult search(Position& pos, int depth) {
    auto search_stack = SearchStack::new_search_stack();
    auto search_globals = SearchGlobals::new_search_globals();
    int alpha = -INFINITY;
    int beta = +INFINITY;
    SearchResult search_result =
        search_impl(pos, alpha, beta, depth, search_stack.begin(), search_globals);
    return search_result;
}

std::optional<Move> best_move_search(Position& pos, SearchGlobals& search_globals) {
    std::optional<Move> best_move;
    auto start_time = curr_time();
    search_globals.set_stop_flag(false);
    search_globals.set_side_to_move(pos.side_to_move());
    search_globals.reset_nodes();
    search_globals.set_start_time(start_time);
    for (int depth = 1; depth <= MAX_PLY; ++depth) {
        auto search_result = search(pos, search_globals, depth);

        if (depth > 1 && search_globals.stop()) {
            return best_move;
        }

        auto time_diff = curr_time() - start_time;

        int score = search_result.score;
        auto& pv = search_result.pv;
        if (!pv) {
            break;
        }

        best_move = *pv->begin();

        UCIScore uci_score = [score]() {
            if (score <= -MAX_MATE_SCORE) {
                return UCIScore{(-score - MATE_SCORE) / 2, UCIScore::ScoreType::MATE};
            } else if (score >= MAX_MATE_SCORE) {
                return UCIScore{(-score + MATE_SCORE + 1) / 2, UCIScore::ScoreType::MATE};
            } else {
                return UCIScore{score, UCIScore::ScoreType::CENTIPAWNS};
            }
        }();

        std::uint64_t time_taken = time_diff.count();
        std::uint64_t nodes = search_globals.nodes();
        std::uint64_t nps = time_taken ? nodes * 1000 / time_taken : nodes;
        UCIInfoParameters info_parameters{{
            {"depth", depth},
            {"score", uci_score},
            {"time", int(time_taken)},
            {"nps", nps},
            {"nodes", nodes},
        }};

        std::vector<std::string> str_move_list;
        str_move_list.reserve(pv->size());
        for (auto move : *pv) {
            str_move_list.push_back(move.to_str());
        }
        info_parameters.set_pv(UCIMoveList{str_move_list});
        UCIService::info(info_parameters);
    }

    return best_move;
}

} // namespace search

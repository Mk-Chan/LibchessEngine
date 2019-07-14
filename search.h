#ifndef SEARCH_H
#define SEARCH_H

#include "libchess/Position.h"
#include "libchess/UCIService.h"

namespace search {

static const int MAX_PLY = 128;
static const int INFINITY = 30001;
static const int MATE_SCORE = 30000;
static const int MAX_MATE_SCORE = MATE_SCORE - MAX_PLY;

static inline std::chrono::milliseconds curr_time() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::high_resolution_clock::now().time_since_epoch());
}

struct SearchResult {
    SearchResult(int score_, std::optional<libchess::MoveList> pv_) noexcept
        : score(score_), pv(std::move(pv_)) {}

    SearchResult operator-() const noexcept { return {-score, pv}; }

    int score;
    std::optional<libchess::MoveList> pv;
};

class SearchGlobals {
  public:
    SearchGlobals(uint64_t nodes, std::optional<std::chrono::milliseconds> start_time,
                  std::optional<libchess::UCIGoParameters> go_parameters) noexcept
        : side_to_move_(libchess::constants::WHITE), stop_flag_(false), nodes_(nodes),
          start_time_(start_time), go_parameters_(std::move(go_parameters)) {}

    [[nodiscard]] std::uint64_t nodes() const noexcept { return nodes_; }
    [[nodiscard]] const std::optional<libchess::UCIGoParameters>& go_parameters() const noexcept {
        return go_parameters_;
    }

    void reset_nodes() noexcept { nodes_ = 0; }
    void set_start_time(std::chrono::milliseconds start_time) noexcept { start_time_ = start_time; }
    void set_go_parameters(const libchess::UCIGoParameters& go_parameters) noexcept {
        go_parameters_ = go_parameters;
    }
    void set_stop_flag(bool stop_flag) noexcept { stop_flag_ = stop_flag; }
    void set_side_to_move(libchess::Color color) noexcept { side_to_move_ = color; }

    static SearchGlobals new_search_globals(
        const std::optional<std::chrono::milliseconds>& start_time = {},
        const std::optional<libchess::UCIGoParameters>& go_parameters = {}) noexcept {
        return SearchGlobals{0, start_time, go_parameters};
    }

    void increment_nodes() noexcept { ++nodes_; }
    [[nodiscard]] bool stop() noexcept {
        if (stop_flag_) {
            return true;
        }
        if (!go_parameters_) {
            return false;
        }
        if (!(nodes_ & 4095U) && start_time_) {
            auto time_diff = curr_time().count() - start_time_->count();

            auto time = [this]() {
                if (side_to_move_ == libchess::constants::WHITE) {
                    return go_parameters_->wtime();
                } else {
                    return go_parameters_->btime();
                }
            }();
            auto inc = [this]() {
                if (side_to_move_ == libchess::constants::WHITE) {
                    return go_parameters_->winc();
                } else {
                    return go_parameters_->binc();
                }
            }();
            auto movestogo = go_parameters_->movestogo();
            if (!movestogo) {
                movestogo = {30};
            }

            if (go_parameters_->infinite()) {
                return false;
            } else if (time && inc) {
                long end_time = (*time + (*movestogo - 1) * *inc) / *movestogo;
                if (*movestogo == 1) {
                    end_time -= 50;
                }
                if (curr_time().count() - start_time_->count() >= end_time) {
                    stop_flag_ = true;
                }
            } else if (go_parameters_->movetime() && time_diff >= go_parameters_->movetime()) {
                stop_flag_ = true;
            }
        }

        return stop_flag_;
    }

  private:
    libchess::Color side_to_move_;
    std::atomic<bool> stop_flag_;
    std::atomic<std::uint64_t> nodes_;
    std::optional<std::chrono::milliseconds> start_time_;
    std::optional<libchess::UCIGoParameters> go_parameters_;
};

int qsearch(libchess::Position&);
SearchResult search(libchess::Position&, int depth);
std::optional<libchess::Move> best_move_search(libchess::Position&, SearchGlobals& search_globals);

} // namespace search

#endif // SEARCH_H

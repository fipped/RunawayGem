#include "runawayGem.hpp"

namespace runawayGem {

int evaluateState(State state, string player) {
    // Simple evaluate
    int res = 0;
    for (auto &gem : state.players[player].gems) {
        res += gem.second;
        if (gem.first == GOLD) {
            res += gem.second * 0.2;
        }
    }
    for (auto &b : state.players[player].bonus) {
        res += b.second;
    }
    return res;
}

vector<MovePtr> getPossibleMove(State state) {
    vector<MovePtr> all_moves;
    // TODO
    return all_moves;
}

int calFinalFitness(const Fitness &fits, string player_name) {
    if (fits.size() == 0)
        return -__INT32_MAX__;
    int ans = 0;
    for (auto &f : fits) {
        ans -= f.second;
    }
    return ans + fits.at(player_name) * 2;
}

Fitness search(const State &state, int depth, string player_name) {
    Fitness all_fitness;
    if (depth == MAX_DEPTH) {
        for (const auto &player : state.players) {
            all_fitness[player.first] = evaluateState(state, player.first);
        }
        return all_fitness;
    }

    // OPTIONAL TODO: PRUNE

    int max_fitness = 0;
    vector<MovePtr> moves = getPossibleMove(state);
    for (auto &mv : moves) {
        State new_state = state;
        mv->move(new_state);
        // 玩家考虑 采取操作 mv 之后，下家采取最佳策略后，所有人的适应度为 fits
        Fitness fits = search(new_state, depth + 1, new_state.player_name);
        // 计算出该得分对于自己来说
        int my_fitness = calFinalFitness(fits, player_name);
        if (my_fitness > max_fitness) {
            all_fitness = fits;
            max_fitness = my_fitness;
        }
    }
    // 采取可令自己最佳的操作后，所有人的适应度
    return all_fitness;
}

MovePtr findNextMove(const State &state) {
    MovePtr best_move;

    int max_score = 0;
    vector<MovePtr> moves = getPossibleMove(state);
    for (MovePtr &mv : moves) {
        Fitness score = search(state, 0, state.player_name);
        //TODO: 用search试每种走法的最终受益，取最大的行动
        if (score[state.player_name] > max_score) {
          max_score = score[state.player_name];
          best_move.reset(mv.release());
        }
    }
    return best_move;
}

vector<Noble> appreciateNobles(const State& state) {
  vector<Noble> appreciate_nobles;
  for(auto noble: state.table.nobles){
    if(noble.score(state.players.at(state.player_name).gems)) {
      appreciate_nobles.push_back(noble);
    }
  }
  return appreciate_nobles;
}
} // namespace runawayGem

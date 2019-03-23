#include "runawayGem.hpp"

namespace runawayGem {

bool ifCanAfford (const Gems & ori_gems, const Gems & bonus, const Card & card) {
    int golden = ori_gems.at(GOLD);
    Gems gems;
    for (auto a : ori_gems) {
        gems[a.first] = a.second;
    }
    for (auto a : bonus) {
        gems[a.first] += a.second;
    }
    for (auto a : card.costs) {
        if (a.second > gems[a.first]) {
            golden -= (a.second - gems[a.first]);
            if (golden < 0) { return false; }
        }
    }
    return true;
}

void getAllCombination(vector<vector<Color> > & result, int start_idx, vector<Color> now_vec) {
    if (now_vec.size() == 3) {
        result.push_back(now_vec);
        return;
    }
    if (5 - start_idx < 3 - now_vec.size()) {
        return;
    }
    for (int i = start_idx; i < 5; i++) {
        now_vec.push_back((Color) i);
        getAllCombination(result, i+1, now_vec);

    }
}

bool FetchSameColor(const Gems& gems, Color color) {
	return gems.at(color) >= 4;
}

bool FetchDiffColor(const Gems& gems, const vector<Color> & colors) {
	for(auto& color : colors)
		if(gems.at(color) == 0)
			return false;
	return true;
}


vector<MovePtr> getPossibleMove(State state) {
    const int MAX_GEMS_NUM = 10;
    vector<MovePtr> all_moves;
    vector<vector<Color> > allColors; getAllCombination(allColors, 0, vector<Color>());
    // state.players[state.player_name].
    const Gems & player_gems = state.players[state.player_name].gems;
    const Gems & player_bonus = state.players[state.player_name].bonus;
    const Gems & table_gems = state.table.gems;
    const vector<Card> table_cards = state.table.cards;
    const vector<Card> reserved_cards = state.players[state.player_name].reserved_cards;
    int player_gem_num = 0;
    for (auto a : player_gems) {
        player_gem_num += a.second;
    }
    // 3 different color
    if (player_gem_num + 3 <= MAX_GEMS_NUM) {
        for (auto col : allColors) {
            if (FetchDiffColor(table_gems, col)) {
                all_moves.push_back(MovePtr(new GetDiffColorGems(col[0], col[1], col[2])));
            }
        }
    }
    // 2 same color
    if (player_gem_num + 2 <= MAX_GEMS_NUM) {
        for (int i = 0; i < 5; i++) {
            Color c = (Color)i;
            if (FetchSameColor(table_gems, c)) {
                all_moves.push_back(MovePtr(new GetTwoSameColorGems(c)));
            }
        }
    }
    // 1 golden & save 1 card
    // only the cards on table are considered
    // TODO: reserve a unknown card
    for (int i = 0; i < table_cards.size(); i++) {
        all_moves.push_back(MovePtr(new ReserveCard(table_cards[i], i)));
    }
    // buy table card
    for (int i = 0; i < table_cards.size(); i++) {
        if (ifCanAfford(player_gems, player_bonus, table_cards[i])) {
            all_moves.push_back(MovePtr(new PurchaseCard(table_cards[i], i)));
        }
    }
    // bug saved card
    for (int i = 0; i < reserved_cards.size(); i++) {
        if (ifCanAfford(player_gems, player_bonus, reserved_cards[i])) {
            all_moves.push_back(MovePtr(new PurchaseReservedCard(reserved_cards[i], i)));
        }
    }

    return all_moves;
}

double evaluateState(State state, string player) {
    double res = 0;
    for (auto &b : state.players[player].bonus) {
        res += b.second;
    }

    for (auto &gem : state.players[player].gems) {
        res += gem.second;
        if (gem.first == GOLD) {
            res += gem.second * 0.2;
        }
    }
    return res;
}



double calFinalFitness(const Fitness &fits, string player_name) {
    if (fits.size() == 0)
        return -__INT32_MAX__;
    double ans = 0;
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

    double max_fitness = 0;
    vector<MovePtr> moves = getPossibleMove(state);
    for (auto &mv : moves) {
        State new_state = state;
        mv->move(new_state);
        // 玩家考虑 采取操作 mv 之后，下家采取最佳策略后，所有人的适应度为 fits
        Fitness fits = search(new_state, depth + 1, new_state.player_name);
        // 计算出该得分对于自己来说
        double my_fitness = calFinalFitness(fits, player_name);
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

    double max_fits = 0;
    vector<MovePtr> moves = getPossibleMove(state);
    for (MovePtr &mv : moves) {
        Fitness fits = search(state, 0, state.player_name);
        //TODO: 用search试每种走法的最终受益，取最大的行动
        if (fits[state.player_name] > max_fits) {
          max_fits = fits[state.player_name];
          best_move.reset(mv.release());
        }
    }
    return best_move;
}

bool appreciateNoble(const State& state, Noble& app_noble) {
  for(auto noble: state.table.nobles){
    if(noble.score(state.players.at(state.player_name).gems)) {
      app_noble = noble;
      return true;
    }
  }
  return false;
}

} // namespace runawayGem

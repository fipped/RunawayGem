#include "runawayGem.hpp"

namespace runawayGem {

bool canAfford(const Gems &ori_gems, const Gems &bonus, const Card &card) {
    int golden = ori_gems.count(GOLD) ? ori_gems.at(GOLD) : 0;
    Gems gems;
    for (auto a : ori_gems) {
        gems[a.first] = a.second;
    }
    for (auto a : bonus) {
      if(gems.count(a.first)){
        gems[a.first] += a.second;
      } else {
        gems[a.first] = a.second;
      }
    }
    for (auto a : card.costs) {
        if (a.second > gems[a.first]) {
            golden -= (a.second - gems[a.first]);
            if (golden < 0) {
                return false;
            }
        }
    }
    return true;
}

void getDifferentColorCombinations(vector<vector<Color>> &result, int start_idx, vector<Color> now_vec) {
    if (now_vec.size() == MAX_DIFFRENT_COLOR) {
        result.push_back(now_vec);
        return;
    }
    if (COLORS_NUM - start_idx < MAX_DIFFRENT_COLOR - now_vec.size()) {
        return;
    }
    for (int i = start_idx; i < COLORS_NUM; i++) {
        now_vec.push_back((Color)i);
        getDifferentColorCombinations(result, i + 1, now_vec);
        now_vec.pop_back();
    }
}

bool FetchSameColor(const Gems &gems, Color color) {
    return gems.count(color) && gems.at(color) >= 4;
}

bool FetchDiffColor(const Gems &gems, const vector<Color> &colors) {
    for (auto &color : colors)
        if (!gems.count(color) || gems.at(color) == 0)
            return false;
    return true;
}

void getPossibleMove(State state, vector<MovePtr>& all_moves) {
    const int MAX_GEMS_NUM = 10;
    vector<vector<Color>> allColors;
    
    getDifferentColorCombinations(allColors, 0, vector<Color>());
    // state.players[state.player_name].
    const Gems &player_gems = state.players[state.player_name].gems;
    const Gems &player_bonus = state.players[state.player_name].bonus;
    const Gems &table_gems = state.table.gems;
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
                all_moves.emplace_back(new GetDiffColorGems(col[0], col[1], col[2]));
            }
        }
    }
    // 2 same color
    if (player_gem_num + 2 <= MAX_GEMS_NUM) {
        for (int i = 0; i < COLORS_NUM; i++) {
            Color c = (Color)i;
            if (FetchSameColor(table_gems, c)) {
                all_moves.emplace_back(new GetTwoSameColorGems(c));
            }
        }
    }
    // 1 golden & save 1 card
    // only the cards on table are considered
    // TODO: reserve a unknown card
    if(player_gem_num + 1 <= MAX_GEMS_NUM && 
      state.table.gems.count(GOLD) && state.table.gems.at(GOLD) > 0) {
      for (int i = 0; i < table_cards.size(); i++) {
          all_moves.emplace_back(new ReserveCard(table_cards[i], i));
      }
    }
    // buy table card
    for (int i = 0; i < table_cards.size(); i++) {
        if (canAfford(player_gems, player_bonus, table_cards[i])) {
            all_moves.emplace_back(new PurchaseCard(table_cards[i], i));
        }
    }
    // bug saved card
    for (int i = 0; i < reserved_cards.size(); i++) {
        if (canAfford(player_gems, player_bonus, reserved_cards[i])) {
            all_moves.emplace_back(new PurchaseReservedCard(reserved_cards[i], i));
        }
    }
}

// TODO: 加上potential收益
double evaluateState(State state, string player) {
    // Simple evaluate
    // params
    const double WEIGHT_SCORE = 100;
    const double WEIGHT_BONUS = 10;
    const double WEIGHT_GEMS = 1;
    const double WEIGHT_GOLD_PLUS = 0.2;

    const double POTENTIAL_AVG_FITNESS = 0.5;        //潜在可购买卡片的收益; 额外core/bonus/收益 / 差的GEM数量 * eight (?)
    const double POTENTIAL_AVG_FITNESS_RESERVED = 1; // 保留卡不会被别人抢先购买
    // 买潜在cards花费更多的bonus更少的gems会更好？

    double res = 0;
    for (auto &gem : state.players[player].gems) {
        if (gem.first == GOLD) {
            res += gem.second * WEIGHT_GOLD_PLUS;
        }
    }
    for (auto &b : state.players[player].bonus) {
        res += b.second * WEIGHT_BONUS;
    }

    res += state.players[player].score * WEIGHT_SCORE;

    return res;
}

double calFinalFitness(const Fitness &fits, string player_name) {
    if (fits.size() == 0)
        return 0;
    double ans = MAX_FIT;
    for (auto &f : fits) {
        ans -=  f.second;
    }
    return ans + fits.at(player_name) * 2;
}

Fitness search(const State &state, int depth, string player_name) {
    static int vistors = 0;
    vistors++;
    Fitness all_fitness;
    if (vistors > 10000 || depth == MAX_DEPTH) {
        for (const auto &player : state.players) {
            all_fitness[player.first] = evaluateState(state, player.first);
        }
        return all_fitness;
    }

    // OPTIONAL TODO: PRUNE

    double max_fitness = 0;
    vector<MovePtr> moves;
    getPossibleMove(state, moves);

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
    double max_fits = 0;
    vector<MovePtr> moves;

    getPossibleMove(state, moves);
    MovePtr best_move(new EmptyMove);
    for (auto& mv : moves) {
        Fitness fits = search(state, 0, state.player_name);
        //TODO: 用search试每种走法的最终受益，取最大的行动
        if (fits[state.player_name] > max_fits) {
            max_fits = fits[state.player_name];
            best_move.reset(mv.release());
        }
    }
    return best_move;
}

bool appreciateNoble(const State &state, Noble &app_noble) {
    for (auto noble : state.table.nobles) {
        if (noble.score(state.players.at(state.player_name).gems)) {
            app_noble = noble;
            return true;
        }
    }
    return false;
}

} // namespace runawayGem

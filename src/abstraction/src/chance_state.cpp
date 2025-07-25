#include "abstraction/chance_state.h"

namespace poker {
ChanceState::ChanceState() : State() {
  for (auto i = 0; i < Global::nofPlayers; ++i) {
    players[i].stack = Global::buyIn;
    players[i].lastAction = Action::None;
  }
  players[0].bet = Global::SB;
  players[1].bet = Global::BB;
  players[0].stack = Global::buyIn - Global::SB;
  players[1].stack = Global::buyIn - Global::BB;
  community.bettingRound = BettingRound::Preflop;
  community.lastPlayer = 1; // initially the BB player is last to act
  community.minRaise = Global::BB;
}

ChanceState::ChanceState(CommunityInfo &community, vector<PlayerInfo> &players,
                         vector<Action> &history)
    : State(community, players, history) {}

void ChanceState::CreateChildren() {
  if (children.size() != 0)
    return;

  auto newPlayers = vector<PlayerInfo>(players);
  auto newCommunity = CommunityInfo(community);

  DealCards(newCommunity, newPlayers);
  newCommunity.minRaise = Global::BB;

  if (GetNumberOfPlayersThatNeedToAct() >= 2) {
    if (community.bettingRound > BettingRound::Preflop) {
      newCommunity.playerToMove = FirstActivePlayer();
      newCommunity.lastPlayer = Global::nofPlayers - 1;
    }
    children.push_back(
        make_shared<PlayState>(newCommunity, newPlayers, history));
  } else {
    if (community.bettingRound < BettingRound::River &&
        GetNumberOfAllInPlayers() >= 2) {
      // directly go to next chance node
      ++newCommunity.bettingRound;
      children.push_back(
          make_shared<ChanceState>(newCommunity, newPlayers, history));
    } else {
      children.push_back(
          make_shared<TerminalState>(newCommunity, newPlayers, history));
    }
  }
}

void ChanceState::DealCards(CommunityInfo &newCommunity,
                            vector<PlayerInfo> &newPlayers) {
  switch (community.bettingRound) {
  case BettingRound::Preflop:
    Global::deck.Shuffle();
    for (auto i = 0; i < Global::nofPlayers; ++i) {
      newPlayers[i].cards = {Global::deck.Peek(i * 2),
                             Global::deck.Peek(i * 2 + 1)};
    }
    break;
  case BettingRound::Flop:
    newCommunity.cards.push_back(Global::deck.Peek(Global::nofPlayers * 2 + 0));
    newCommunity.cards.push_back(Global::deck.Peek(Global::nofPlayers * 2 + 1));
    newCommunity.cards.push_back(Global::deck.Peek(Global::nofPlayers * 2 + 2));
    break;
  case BettingRound::Turn:
    newCommunity.cards.push_back(Global::deck.Peek(Global::nofPlayers * 2 + 3));
    break;
  case BettingRound::River:
    newCommunity.cards.push_back(Global::deck.Peek(Global::nofPlayers * 2 + 4));
    break;
  default:
    throw invalid_argument("Unknown betting round");
  }
}

/// <summary>
/// Note: The single child was already randomly created
/// </summary>
/// <returns></returns>
shared_ptr<State> ChanceState::DoRandomAction() {
  CreateChildren();
  return children[0];
}

vector<shared_ptr<PlayState>> ChanceState::GetFirstActionStates() {
  auto gameStates = vector<shared_ptr<PlayState>>();

  auto newCommunity = CommunityInfo(community);
  // create one playstate child after chance
  int lastToMoveTemp = -1;
  int minRaiseTemp = Global::BB;
  auto bettingRound = community.bettingRound;

  if (community.bettingRound == 0) {
    for (auto i = 2 % Global::nofPlayers;; i = (i + 1) % Global::nofPlayers) {
      if (players[i].IsAlive() && players[i].lastAction != Action::Allin) {
        lastToMoveTemp = i;
      }
      if (i == 1)
        break;
    }
  } else if (community.bettingRound > 0) {
    for (auto i = 0; i < Global::nofPlayers; ++i) {
      if (players[i].IsAlive() && players[i].stack != 0) {
        lastToMoveTemp = i;
      }
    }
  }

  vector<Hand> startingHands = utils::GetStartingHandChart();

  for (auto i = 0; i < Global::RANKS * Global::RANKS; ++i) {
    auto newPlayers = vector<PlayerInfo>(players);
    newPlayers[2 % Global::nofPlayers].cards = {
        startingHands[i].cards[0].Bitmask(),
        startingHands[i].cards[1].Bitmask()};
    newCommunity.cards = vector<ulong>();
    newCommunity.lastPlayer = lastToMoveTemp;
    newCommunity.minRaise = minRaiseTemp;
    newCommunity.bettingRound = bettingRound;
    gameStates.push_back(
        make_shared<PlayState>(newCommunity, newPlayers, history));
  }

  return gameStates;
}

bool ChanceState::IsPlayerInHand(int player) {
  return players[player].IsAlive();
}

ostream &ChanceState::Print(ostream &out) const {
  out << "Chance | ";
  return State::Print(out);
}
} // namespace poker

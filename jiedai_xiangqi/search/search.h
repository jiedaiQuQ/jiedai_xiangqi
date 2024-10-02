#pragma once

#include <bits/stdc++.h>

#include "jiedai_xiangqi\board\board.h"
#include "jiedai_xiangqi\utils\const.h"

using namespace std;

ChessBoard chessBoard;
ChessBoard board;
Move computerMove;
long long nodeCount, leafCount, hitCount, quiesCount, evaluateCount;

int Quies(int alpha, int beta, int distance) {
  quiesCount++;
  if (-WinValue + distance >= beta) return beta;  // 无害裁剪
  int value = board.Evaluate1();
  if (value >= beta) {
    return beta;
  }
  if (value > alpha) {
    alpha = value;
  }
  vector<Move> moves = board.GenerateEatMoves(board.color);
  static int mvvlva[256][256];
  for (Move& move : moves) mvvlva[move.begin][move.end] = board.MVVLVA(move);
  sort(moves.begin(), moves.end(), [&](Move& a, Move& b) {
    if (mvvlva[a.begin][a.end] != mvvlva[b.begin][b.end])
      return mvvlva[a.begin][a.end] > mvvlva[b.begin][b.end];
    static int simpleValue[8] = {0, 5, 1, 1, 3, 4, 3, 2};
    return simpleValue[a.kill] > simpleValue[b.kill];
  });
  while (moves.size() && mvvlva[moves.back().begin][moves.back().end] == -1)
    moves.pop_back();
  for (auto& move : moves) {
    board.ExecuteMove(move);
    if (board.CheckKill(!board.color)) {
      board.RescindMove(move);
    } else {
      value = -Quies(-beta, -alpha, distance + 1);
      board.RescindMove(move);
      if (value >= beta) {
        return beta;
      }
      if (value > alpha) {
        alpha = value;
      }
    }
  }
  return alpha;
}

int Evaluate(int distance) {
  evaluateCount++;
  //	return board.Evaluate1();
  return Quies(-Infinite, Infinite, distance);
}

int AlphaBetaNonRoot(int alpha, int beta, int depth, int distance,
                     bool canNullMove, deque<Move>& moveTrace) {
  nodeCount++;
  if (-WinValue + distance >= beta) return beta;  // 无害裁剪
  Move bestMove = {0};
  Move imagineMove = {0};
  TTItem* item = transpositionTable.Query(board.key);
  if (item != NULL) {
    if (item->depth >= depth) {
      int minValue = item->minValue;
      int maxValue = item->maxValue;
      if (minValue == maxValue) {
        hitCount++;
        if (minValue >= WinCheck)
          minValue -= distance;
        else if (minValue <= -WinCheck)
          minValue += distance;
        return minValue;
      }
      if (minValue >= beta) {
        hitCount++;
        return beta;
      }
      if (maxValue <= alpha) {
        hitCount++;
        return alpha;
      }
    }
    imagineMove = (Move){item->begin, item->end, board.chess[item->end].type};
  }
  if (depth == 0) {
    leafCount++;
    int value = Evaluate(distance);
    transpositionTable.Insert(board.key, value, value, depth, distance,
                              bestMove);  // 更新置换表
    return value;
  }
  if (canNullMove && depth >= 3 && !board.CheckKill(board.color)) {  // 空着
    board.ExecuteNullMove();
    deque<Move> trace;
    int value = -AlphaBetaNonRoot(-beta, -beta + 1, depth - 1 - 2, distance + 1,
                                  false, trace);
    board.RescindNullMove();
    if (value >= beta) {
      if (board.NullMoveSafe()) return beta;
      if (AlphaBetaNonRoot(beta - 1, beta, depth - 2, distance, false, trace) >=
          beta)
        return beta;  // 空着检验
    }
  }
  if (depth >= 3 && imagineMove.begin == 0 &&
      beta - alpha > 1) {  // 内部迭代加深
    deque<Move> trace;
    int value =
        AlphaBetaNonRoot(alpha, beta, depth / 2, distance, false, trace);
    if (value <= alpha) {
      value =
          AlphaBetaNonRoot(-Infinite, beta, depth / 2, distance, false, trace);
    }
    item = transpositionTable.Query(board.key);
    if (item != NULL)
      imagineMove = (Move){item->begin, item->end, board.chess[item->end].type};
  }

  vector<Move> moves;
  vector<Move> eatMoves;
  int state = 0;
  int index = 0;
  Move killerMove1 = {0};
  Move killerMove2 = {0};
  bool canMove = false;
  bool isPV = false;

  function<Move()> NextMove = [&]() {
    if (state == 0) {
      state = 1;
      if (imagineMove.begin) return imagineMove;
    }
    if (state == 1) {
      state = 2;
      index = 0;
      eatMoves = board.GenerateEatMoves(board.color);
      static int mvvlva[256][256];
      for (Move& move : eatMoves)
        mvvlva[move.begin][move.end] = board.MVVLVA(move);
      sort(eatMoves.begin(), eatMoves.end(), [&](Move& a, Move& b) {
        return mvvlva[a.begin][a.end] > mvvlva[b.begin][b.end];
      });  // 根据MVV(LVA)值排序
      while (eatMoves.size() &&
             mvvlva[eatMoves.back().begin][eatMoves.back().end] <= 0)
        eatMoves.pop_back();
    }
    if (state == 2) {
      while (index < eatMoves.size()) {
        Move& move = eatMoves[index++];
        if (move.begin == imagineMove.begin && move.end == imagineMove.end)
          continue;
        if (move.begin == killerMove1.begin && move.end == killerMove1.end)
          continue;
        if (move.begin == killerMove2.begin && move.end == killerMove2.end)
          continue;
        return move;
      }
      state = 3;
    }
    if (state == 3) {
      state = 4;
      killerMove1 = killerTable.killer1[distance];
      killerMove1.kill = board.chess[killerMove1.end].type;
      if (board.chess[killerMove1.begin].color == board.color)
        if (killerMove1.begin != imagineMove.begin ||
            killerMove1.end != imagineMove.end)
          if (board.CheckMove(killerMove1.begin, killerMove1.end))
            return killerMove1;
    }
    if (state == 4) {
      state = 5;
      killerMove2 = killerTable.killer2[distance];
      killerMove2.kill = board.chess[killerMove2.end].type;
      if (board.chess[killerMove2.begin].color == board.color)
        if (killerMove2.begin != imagineMove.begin ||
            killerMove2.end != imagineMove.end)
          if (board.CheckMove(killerMove2.begin, killerMove2.end))
            return killerMove2;
    }
    if (state == 5) {
      state = 6;
      index = 0;
      moves = board.GenerateAllMoves(board.color);
      static bool moved[256][256];
      for (Move& move : moves) moved[move.begin][move.end] = 0;
      for (Move& move : eatMoves) moved[move.begin][move.end] = 1;
      moved[imagineMove.begin][imagineMove.end] = 1;
      moved[killerMove1.begin][killerMove1.end] = 1;
      moved[killerMove2.begin][killerMove2.end] = 1;
      for (int i = 0; i < moves.size();) {
        if (moved[moves[i].begin][moves[i].end]) {
          swap(moves[i], moves.back());
          moves.pop_back();
        } else
          i++;
      }
      sort(moves.begin(), moves.end(), [&](Move& a, Move& b) {
        return historyTable.value[a.begin][a.end] >
               historyTable.value[b.begin][b.end];
      });  // 根据历史表排序
    }
    if (state == 6) {
      while (index < moves.size()) return moves[index++];
    }
    return Move{0};
  };

  for (Move move = NextMove(); move.begin; move = NextMove()) {
    board.ExecuteMove(move);
    if (board.CheckKill(!board.color)) {
      board.RescindMove(move);
    } else {
      int value;
      deque<Move> trace;
      int repeatCount = board.repeatTable.Query(board.key);
      if (repeatCount > 1) {
        int repeatState = 1;
        if (board.CheckKill(board.color)) repeatState += 2;
        board.RescindMove(move);
        if (board.CheckKill(board.color)) repeatState += 4;
        if (repeatState == 3)
          value = -BanValue;
        else if (repeatState == 5)
          value = BanValue;
        else
          value = DrawValue;
      } else {
        int nextDepth = depth - 1;
        if (board.CheckKill(board.color)) nextDepth = depth;
        if (canMove) {
          value = -AlphaBetaNonRoot(-alpha - 1, -alpha, nextDepth, distance + 1,
                                    canNullMove, trace);
          if (value > alpha && value < beta) {
            value = -AlphaBetaNonRoot(-beta, -alpha, nextDepth, distance + 1,
                                      canNullMove, trace);
          }
        } else {
          value = -AlphaBetaNonRoot(-beta, -alpha, nextDepth, distance + 1,
                                    canNullMove, trace);
          canMove = true;
        }
        board.RescindMove(move);
      }
      if (value >= beta) {
        historyTable.Update(move, depth);    // 更新历史表
        killerTable.Insert(move, distance);  // 更新杀手表
        if (value != BanValue)
          transpositionTable.Insert(board.key, value, Infinite, depth, distance,
                                    move);  // 更新置换表
        return beta;
      }
      if (value > alpha) {
        moveTrace = trace;
        isPV = true;
        bestMove = move;
        alpha = value;
      }
    }
  }
  if (!canMove) {
    return -WinValue + distance;
  }
  historyTable.Update(bestMove, depth);  // 更新历史表
  if (alpha != BanValue) {
    if (isPV)
      transpositionTable.Insert(board.key, alpha, alpha, depth, distance,
                                bestMove);  // 更新置换表
    else
      transpositionTable.Insert(board.key, -Infinite, alpha, depth, distance,
                                bestMove);  // 更新置换表
  }
  moveTrace.push_front(bestMove);
  return alpha;
}

int AlphaBetaRoot(int alpha, int beta, int depth, deque<Move>& moveTrace,
                  vector<Move>& moves) {
  nodeCount++;
  Move bestMove = {0};
  bool canMove = false;
  bool isPV = false;

  if (moves.empty()) {
    moves = board.GenerateAllMoves(board.color);
    sort(moves.begin(), moves.end(), [&](Move& a, Move& b) {
      if (board.MVVLVA(a) > 0 || board.MVVLVA(b) > 0)
        return board.MVVLVA(a) > board.MVVLVA(b);
      return historyTable.value[a.begin][a.end] >
             historyTable.value[b.begin][b.end];
    });  // 根据历史表排序
  }

  static int moveValue[256][256];
  for (int i = 0; i < moves.size(); i++) {
    Move& move = moves[i];
    moveValue[move.begin][move.end] = -Infinite - i;
  }
  for (Move& move : moves) {
    // Timer timer("depth "+to_string(depth));
    // if(depth<7)timer.Shutdown();
    board.ExecuteMove(move);
    if (board.CheckKill(!board.color)) {
      board.RescindMove(move);
    } else {
      int value;
      deque<Move> trace;
      int repeatCount = board.repeatTable.Query(board.key);
      if (repeatCount > 1) {
        int repeatState = 1;
        if (board.CheckKill(board.color)) repeatState += 2;
        board.RescindMove(move);
        if (board.CheckKill(board.color)) repeatState += 4;
        if (repeatState == 3)
          value = -BanValue;
        else if (repeatState == 5 && repeatCount >= 3)
          value = BanValue;
        else
          value = DrawValue;
      } else {
        int nextDepth = depth - 1;
        if (board.CheckKill(board.color)) nextDepth = depth;
        if (canMove) {
          value =
              -AlphaBetaNonRoot(-alpha - 1, -alpha, nextDepth, 1, true, trace);
          if (value > alpha && value < beta) {
            value = -AlphaBetaNonRoot(-beta, -alpha, nextDepth, 1, true, trace);
          }
        } else {
          value = -AlphaBetaNonRoot(-beta, -alpha, nextDepth, 1, true, trace);
          canMove = true;
        }
        board.RescindMove(move);
      }
      if (value > alpha) {
        moveValue[move.begin][move.end] = value;
      }
      if (value >= beta) {
        historyTable.Update(bestMove, depth);  // 更新历史表
        if (value != BanValue)
          transpositionTable.Insert(board.key, value, Infinite, depth, 0,
                                    move);  // 更新置换表
        sort(moves.begin(), moves.end(), [&](Move& a, Move& b) {
          if (moveValue[a.begin][a.end] != moveValue[b.begin][b.end])
            return moveValue[a.begin][a.end] > moveValue[b.begin][b.end];
          if (board.MVVLVA(a) > 0 || board.MVVLVA(b) > 0)
            return board.MVVLVA(a) > board.MVVLVA(b);
          return historyTable.value[a.begin][a.end] >
                 historyTable.value[b.begin][b.end];
        });  // 更新根节点着法顺序
        return beta;
      }
      if (value > alpha) {
        moveTrace = trace;
        isPV = true;
        bestMove = move;
        computerMove = move;
        alpha = value;
      }
    }
  }
  if (!canMove) {
    return -WinValue;
  }
  historyTable.Update(bestMove, depth);  // 更新历史表
  if (alpha != BanValue) {
    if (isPV)
      transpositionTable.Insert(board.key, alpha, alpha, depth, 0,
                                bestMove);  // 更新置换表
    else
      transpositionTable.Insert(board.key, -Infinite, alpha, depth, 0,
                                bestMove);  // 更新置换表
  }
  moveTrace.push_front(bestMove);
  sort(moves.begin(), moves.end(), [&](Move& a, Move& b) {
    if (moveValue[a.begin][a.end] != moveValue[b.begin][b.end])
      return moveValue[a.begin][a.end] > moveValue[b.begin][b.end];
    return historyTable.value[a.begin][a.end] >
           historyTable.value[b.begin][b.end];
  });  // 更新根节点着法顺序
  return alpha;
}

Move CalculationComputerMove(int level, int step) {
  board = chessBoard;
  historyTable.Attenuate();  // 每次搜索前衰减历史表
  killerTable.Init();        // 每次搜索前清空杀手表
  nodeCount = 0;
  leafCount = 0;
  hitCount = 0;
  computerMove = {0};
  deque<Move> moveTrace;

  int t1 = clock();
  printf("------------------------------------------\n");
  printf("start calc...\n");

  int value = 0;
  vector<Move> moves;
  for (int depth = (level - 1) % step + 1; depth <= level; depth += step) {
    int window = 60;
    //		window=Infinite;
    int alpha = value - window;
    int beta = value + window;
    while (1) {
      value = AlphaBetaRoot(alpha, beta, depth, moveTrace, moves);
      printf("depth=%d value=%d [%d %d]\n", depth, value, alpha, beta);
      if (value >= WinCheck || value <= -WinCheck)
        break;
      else if (value <= alpha)
        alpha = -Infinite;
      else if (value >= beta)
        beta = Infinite;
      else
        break;
    }
    if (value >= WinCheck || value <= -WinCheck) break;
  }

  int t2 = clock();
  double t = 1.0 * (t2 - t1) / CLOCKS_PER_SEC;
  printf("end calc:\n");
  printf("time = %.3lfs   value = %d\n", t, value);
  printf("node = %lld   leaf = %lld   k = %.0lf\n", nodeCount, leafCount,
         1e8 * t / nodeCount);
  printf("hit = %lld   rate = %.2lf\%\n", hitCount,
         100.0 * hitCount / nodeCount);
  printf("quies = %.2lf\n", 1.0 * quiesCount / evaluateCount);
  static double a = 0, b = 0;
  a += 1.0 * quiesCount / evaluateCount, b += 1;
  printf("quies == %.2lf\n", a / b);
  //	for(auto &move:moveTrace)printf("%x -> %x\n",move.begin,move.end);
  printf("------------------------------------------\n");
  assert(computerMove.begin && computerMove.end);
  return computerMove;
}

void ApiInit() {
  lineSituation.Init();
  for (int i = 4; i <= 7; i++)
    for (int j = 3; j <= 11; j++)
      bingNextPosition[0][PieceColor::kRed][HW(i, j)] = HW(i - 1, j);
  for (int i = 8; i <= 9; i++)
    for (int j = 3; j <= 11; j += 2)
      bingNextPosition[0][PieceColor::kRed][HW(i, j)] = HW(i - 1, j);
  for (int i = 3; i <= 7; i++)
    for (int j = 4; j <= 11; j++)
      bingNextPosition[1][PieceColor::kRed][HW(i, j)] = HW(i, j - 1);
  for (int i = 3; i <= 7; i++)
    for (int j = 3; j <= 10; j++)
      bingNextPosition[2][PieceColor::kRed][HW(i, j)] = HW(i, j + 1);
  for (int i = 8; i <= 11; i++)
    for (int j = 3; j <= 11; j++)
      bingNextPosition[0][PieceColor::kBlack][HW(i, j)] = HW(i + 1, j);
  for (int i = 6; i <= 7; i++)
    for (int j = 3; j <= 11; j += 2)
      bingNextPosition[0][PieceColor::kBlack][HW(i, j)] = HW(i + 1, j);
  for (int i = 8; i <= 12; i++)
    for (int j = 4; j <= 11; j++)
      bingNextPosition[1][PieceColor::kBlack][HW(i, j)] = HW(i, j - 1);
  for (int i = 8; i <= 12; i++)
    for (int j = 3; j <= 10; j++)
      bingNextPosition[2][PieceColor::kBlack][HW(i, j)] = HW(i, j + 1);
  lineupTable.Init();
}
void ApiNewGame(GameMode mode, int level, int step) {
  if (mode == GameMode::kModePlayerPlayer) {
    chessBoard.roleRed = RoleType::kRolePlayer;
    chessBoard.roleBlack = RoleType::kRolePlayer;
    chessBoard.flip = false;
  } else if (mode == GameMode::kModePlayerComputer) {
    chessBoard.roleRed = RoleType::kRolePlayer;
    chessBoard.roleBlack = RoleType::kRoleComputer;
    chessBoard.flip = false;
  } else if (mode == GameMode::kModeComputerPlayer) {
    chessBoard.roleRed = RoleType::kRoleComputer;
    chessBoard.roleBlack = RoleType::kRolePlayer;
    chessBoard.flip = true;
  } else if (mode == GameMode::kModeComputerComputer) {
    chessBoard.roleRed = RoleType::kRoleComputer;
    chessBoard.roleBlack = RoleType::kRoleComputer;
    chessBoard.flip = false;
  } else if (mode == GameMode::kModeFirst) {
    chessBoard.roleRed = RoleType::kRoleComputer;
    chessBoard.roleBlack = RoleType::kRolePlayer;
    chessBoard.flip = false;
  } else if (mode == GameMode::kModeBack) {
    chessBoard.roleRed = RoleType::kRolePlayer;
    chessBoard.roleBlack = RoleType::kRoleComputer;
    chessBoard.flip = true;
  }
  chessBoard.computerLevel = level;
  chessBoard.computerStep = step;
  chessBoard.hintLevel = level;
  chessBoard.hintStep = step;
  chessBoard.Init();
  chessBoard.AddChess(HW(12, 3), PieceColor::kRed, PieceType::kJu);
  chessBoard.AddChess(HW(12, 4), PieceColor::kRed, PieceType::kMa);
  chessBoard.AddChess(HW(12, 5), PieceColor::kRed, PieceType::kXiang);
  chessBoard.AddChess(HW(12, 6), PieceColor::kRed, PieceType::kShi);
  chessBoard.AddChess(HW(12, 7), PieceColor::kRed, PieceType::kShuai);
  chessBoard.AddChess(HW(12, 8), PieceColor::kRed, PieceType::kShi);
  chessBoard.AddChess(HW(12, 9), PieceColor::kRed, PieceType::kXiang);
  chessBoard.AddChess(HW(12, 10), PieceColor::kRed, PieceType::kMa);
  chessBoard.AddChess(HW(12, 11), PieceColor::kRed, PieceType::kJu);
  chessBoard.AddChess(HW(10, 4), PieceColor::kRed, PieceType::kPao);
  chessBoard.AddChess(HW(10, 10), PieceColor::kRed, PieceType::kPao);
  chessBoard.AddChess(HW(9, 3), PieceColor::kRed, PieceType::kBing);
  chessBoard.AddChess(HW(9, 5), PieceColor::kRed, PieceType::kBing);
  chessBoard.AddChess(HW(9, 7), PieceColor::kRed, PieceType::kBing);
  chessBoard.AddChess(HW(9, 9), PieceColor::kRed, PieceType::kBing);
  chessBoard.AddChess(HW(9, 11), PieceColor::kRed, PieceType::kBing);
  chessBoard.AddChess(HW(3, 3), PieceColor::kBlack, PieceType::kJu);
  chessBoard.AddChess(HW(3, 4), PieceColor::kBlack, PieceType::kMa);
  chessBoard.AddChess(HW(3, 5), PieceColor::kBlack, PieceType::kXiang);
  chessBoard.AddChess(HW(3, 6), PieceColor::kBlack, PieceType::kShi);
  chessBoard.AddChess(HW(3, 7), PieceColor::kBlack, PieceType::kShuai);
  chessBoard.AddChess(HW(3, 8), PieceColor::kBlack, PieceType::kShi);
  chessBoard.AddChess(HW(3, 9), PieceColor::kBlack, PieceType::kXiang);
  chessBoard.AddChess(HW(3, 10), PieceColor::kBlack, PieceType::kMa);
  chessBoard.AddChess(HW(3, 11), PieceColor::kBlack, PieceType::kJu);
  chessBoard.AddChess(HW(5, 4), PieceColor::kBlack, PieceType::kPao);
  chessBoard.AddChess(HW(5, 10), PieceColor::kBlack, PieceType::kPao);
  chessBoard.AddChess(HW(6, 3), PieceColor::kBlack, PieceType::kBing);
  chessBoard.AddChess(HW(6, 5), PieceColor::kBlack, PieceType::kBing);
  chessBoard.AddChess(HW(6, 7), PieceColor::kBlack, PieceType::kBing);
  chessBoard.AddChess(HW(6, 9), PieceColor::kBlack, PieceType::kBing);
  chessBoard.AddChess(HW(6, 11), PieceColor::kBlack, PieceType::kBing);

  chessBoard.repeatTable.Add(chessBoard.key);

  printf("|----------------------|\n");
  printf("| level = %d   step = %d |\n", chessBoard.computerLevel,
         chessBoard.computerStep);
  printf("|----------------------|\n");
}
void ApiPlace(ChessBoard board) {
  chessBoard.roleRed = board.roleRed;
  chessBoard.roleBlack = board.roleBlack;
  chessBoard.flip = board.flip;
  chessBoard.computerLevel = board.computerLevel;
  chessBoard.computerStep = board.computerStep;
  chessBoard.hintLevel = board.hintLevel;
  chessBoard.hintStep = board.hintStep;
  chessBoard.Init(board.color);

  for (int i = 0; i < 256; i++)
    if (board.chess[i].type)
      chessBoard.AddChess(i, board.chess[i].color, board.chess[i].type);

  chessBoard.repeatTable.Add(chessBoard.key);

  printf("|----------------------|\n");
  printf("| level = %d   step = %d |\n", chessBoard.computerLevel,
         chessBoard.computerStep);
  printf("|----------------------|\n");
}
void ApiSet(RoleType roleRed, RoleType roleBlack, int computerLevel,
            int computerStep, int hintLevel, int hintStep) {
  chessBoard.roleRed = roleRed;
  chessBoard.roleBlack = roleBlack;
  chessBoard.computerLevel = computerLevel;
  chessBoard.computerStep = computerStep;
  chessBoard.hintLevel = hintLevel;
  chessBoard.hintStep = hintStep;
}
void ApiPlayerMove(int begin, int end) {
  Move move = (Move){begin, end, chessBoard.chess[end].type};
  chessBoard.ExecuteMove(move);
  chessBoard.moveStack.push_back(move);
}
void ApiComputerMove() {
  Move move = CalculationComputerMove(chessBoard.computerLevel,
                                      chessBoard.computerStep);
  if (move.begin == 0 && move.end == 0) return;
  chessBoard.ExecuteMove(move);
  chessBoard.moveStack.push_back(move);
  chessBoard.Evaluate1(1);  // debug
}
void ApiHint() {
  Move move =
      CalculationComputerMove(chessBoard.hintLevel, chessBoard.hintStep);
  if (move.begin == 0 && move.end == 0) return;
  chessBoard.ExecuteMove(move);
  chessBoard.moveStack.push_back(move);
  chessBoard.Evaluate1(1);  // debug
}
void ApiRepent() {
  int count = 0;
  if (chessBoard.roleRed == RoleType::kRoleComputer &&
      chessBoard.roleBlack == RoleType::kRoleComputer)
    count = 1;
  else if (chessBoard.color == PieceColor::kRed &&
           chessBoard.roleBlack == RoleType::kRolePlayer)
    count = 1;
  else if (chessBoard.color == PieceColor::kBlack &&
           chessBoard.roleRed == RoleType::kRolePlayer)
    count = 1;
  else
    count = 2;
  while (count > 0 && chessBoard.moveStack.size() > 0) {
    chessBoard.RescindMove(chessBoard.moveStack.back());
    chessBoard.moveStack.pop_back();
    count--;
  }
}
bool ApiCheckLose() { return chessBoard.CheckLose(); }
bool ApiCanMove(int a, int b) {
  int canMove = chessBoard.CheckMove(a, b);
  if (canMove) {
    Move move = (Move){a, b, chessBoard.chess[b].type};
    chessBoard.ExecuteMove(move);
    if (chessBoard.CheckKill(!chessBoard.color)) canMove = false;
    chessBoard.RescindMove(move);
  }
  return canMove;
}

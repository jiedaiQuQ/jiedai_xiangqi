#pragma once
#include <bits/stdc++.h>

#include "jiedai_xiangqi\utils\const.h"

using namespace std;

class Timer {
 public:
  Timer(string message = "")
      : message(message), start_time(chrono::steady_clock::now()) {
    shutdown_ = false;
  }
  ~Timer() {
    if (shutdown_) return;
    auto end_time = chrono::steady_clock::now();
    auto elapsed_time =
        chrono::duration_cast<chrono::duration<double>>(end_time - start_time)
            .count();
    cout << message << " cost " << fixed << setprecision(3) << elapsed_time
         << " s" << endl;
  }
  void Shutdown() { shutdown_ = true; }

 private:
  string message;
  chrono::time_point<chrono::steady_clock> start_time;
  bool shutdown_;
};

PieceColor OtherColor(PieceColor color) {
  return color == PieceColor::kBlack ? PieceColor::kRed : PieceColor::kBlack;
}
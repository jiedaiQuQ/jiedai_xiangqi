#pragma once

#include "graphics.h"

namespace Ctrl {

constexpr int GREY = 7, WHITE = 15, GREEN = 10, BLUE = 11, PURPLE = 13,
              YELLOW = 14, RED = 12;
constexpr int GREEN_ = 2, BLUE_ = 3, PURPLE_ = 5, YELLOW_ = 6, RED_ = 4;

void SetColor(int color) {
  static int nowColor = GREY;
  if (nowColor == color) return;
  nowColor = color;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}
// void SetWindowSize(int wide, int high) {
//   HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
//   SetConsoleScreenBufferSize(hOut, (COORD){wide, 500});
//   SMALL_RECT rc = {0, 0, wide - 1, high - 1};
//   SetConsoleWindowInfo(hOut, true, &rc);
// }

}  // namespace Ctrl
#include <bits/stdc++.h>
using namespace std;

// -------------------------------------------------------
//  INPUT — edit these three lines to change the test case
//  Keep N <= 3 for brute force (8x8 is the practical limit)
// -------------------------------------------------------
const int N          = 2;   // board is 2^N x 2^N  (must be > 1)
const int MISSING_R  = 0;   // row of missing square (0-indexed)
const int MISSING_C  = 0;   // col of missing square (0-indexed)
// -------------------------------------------------------

const int SHAPES[4][3][2] = {
    {{0,0},{0,1},{1,0}},
    {{0,0},{1,0},{1,1}},
    {{0,0},{0,1},{1,1}},
    {{0,1},{1,0},{1,1}}
};

int board[64][64];
int colorBoard[64][64];
int sz;
int trominoID = 0;

bool colorValid(vector<pair<int,int> >& cells, int col) {
    set<pair<int,int> > cellSet(cells.begin(), cells.end());
    int dr[] = {0, 0,  1, -1};
    int dc[] = {1, -1, 0,  0};

    for (int i = 0; i < (int)cells.size(); i++) {
        int r = cells[i].first;
        int c = cells[i].second;
        for (int d = 0; d < 4; d++) {
            int nr = r + dr[d];
            int nc = c + dc[d];
            if (nr < 0 || nr >= sz || nc < 0 || nc >= sz) continue;
            if (board[nr][nc] <= 0)                        continue;
            if (cellSet.count(make_pair(nr, nc)))          continue;
            if (colorBoard[nr][nc] == col)                 return false;
        }
    }
    return true;
}

bool solve(int startR, int startC) {
    int r = -1, c = -1;
    bool found = false;
    for (int row = startR; row < sz && !found; row++) {
        int colStart = (row == startR) ? startC : 0;
        for (int col = colStart; col < sz && !found; col++) {
            if (board[row][col] == 0) {
                r = row; c = col;
                found = true;
            }
        }
    }
    if (!found) return true;

    for (int s = 0; s < 4; s++) {
        for (int k = 0; k < 3; k++) {
            int ar = r - SHAPES[s][k][0];
            int ac = c - SHAPES[s][k][1];

            vector<pair<int,int> > cells;
            bool valid = true;
            for (int j = 0; j < 3; j++) {
                int nr = ar + SHAPES[s][j][0];
                int nc = ac + SHAPES[s][j][1];
                if (nr < 0 || nr >= sz || nc < 0 || nc >= sz || board[nr][nc] != 0) {
                    valid = false;
                    break;
                }
                cells.push_back(make_pair(nr, nc));
            }
            if (!valid) continue;

            trominoID++;
            for (int col = 0; col < 3; col++) {
                if (!colorValid(cells, col)) continue;

                for (int i = 0; i < (int)cells.size(); i++) {
                    board[cells[i].first][cells[i].second]      = trominoID;
                    colorBoard[cells[i].first][cells[i].second] = col;
                }

                if (solve(r, c)) return true;

                for (int i = 0; i < (int)cells.size(); i++) {
                    board[cells[i].first][cells[i].second]      = 0;
                    colorBoard[cells[i].first][cells[i].second] = -1;
                }
            }
            trominoID--;
        }
    }

    return false;
}

int main() {
    int n  = N;
    int mr = MISSING_R;
    int mc = MISSING_C;

    if (n <= 1) {
        printf("Error: N must be > 1 (minimum board is 4x4).\n");
        return 1;
    }

    sz = 1 << n;

    if (sz > 64) {
        printf("Error: N=%d gives a %dx%d board — too large for brute force. Use N <= 3.\n",
               n, sz, sz);
        return 1;
    }

    if (mr < 0 || mr >= sz || mc < 0 || mc >= sz) {
        printf("Error: missing cell (%d,%d) is outside the %dx%d board.\n",
               mr, mc, sz, sz);
        return 1;
    }

    printf("Brute Force  |  n=%d, board=%dx%d, missing=(%d,%d)\n\n",
           n, sz, sz, mr, mc);

    memset(board,      0,  sizeof(board));
    memset(colorBoard, -1, sizeof(colorBoard));
    board[mr][mc] = -1;

    if (solve(0, 0)) {
        printf("Solution found!\n\n");

        string colChar = "RGB";
        for (int r = 0; r < sz; r++) {
            for (int c = 0; c < sz; c++) {
                if (board[r][c] == -1)
                    printf("  .   ");
                else
                    printf("T%02d%c  ", board[r][c], colChar[colorBoard[r][c]]);
            }
            printf("\n");
        }

        bool valid = true;
        int dr[] = {0, 1};
        int dc[] = {1, 0};
        for (int r = 0; r < sz && valid; r++) {
            for (int c = 0; c < sz && valid; c++) {
                if (board[r][c] <= 0) continue;
                for (int d = 0; d < 2; d++) {
                    int nr = r + dr[d], nc = c + dc[d];
                    if (nr >= sz || nc >= sz) continue;
                    if (board[nr][nc] <= 0) continue;
                    if (board[nr][nc] == board[r][c]) continue;
                    if (colorBoard[nr][nc] == colorBoard[r][c]) {
                        valid = false;
                        printf("Color conflict at (%d,%d) and (%d,%d)!\n", r, c, nr, nc);
                    }
                }
            }
        }
        printf("\nColor validity  : %s\n", valid ? "PASSED" : "FAILED");
        printf("Trominoes placed: %d\n", trominoID);
    } else {
        printf("No solution found.\n");
    }

    return 0;
}


#include <bits/stdc++.h>
using namespace std;

// -------------------------------------------------------
//  INPUT — edit these three lines to change the test case
// -------------------------------------------------------
const int N          = 2;   // board is 2^N x 2^N  (must be > 1)
const int MISSING_R  = 0;   // row of missing square (0-indexed)
const int MISSING_C  = 0;   // col of missing square (0-indexed)
// -------------------------------------------------------

int board[512][512];
int colorBoard[512][512];
int trominoID = 0;
int BOARD_SIZE = 0;

bool inBounds(int r, int c) {
    return r >= 0 && c >= 0 && r < BOARD_SIZE && c < BOARD_SIZE;
}

int assignColor(vector<pair<int,int> >& cells) {
    set<pair<int,int> > cellSet(cells.begin(), cells.end());
    set<int> usedColors;
    int dr[] = {0, 0,  1, -1};
    int dc[] = {1, -1, 0,  0};

    for (int i = 0; i < (int)cells.size(); i++) {
        int r = cells[i].first;
        int c = cells[i].second;
        for (int d = 0; d < 4; d++) {
            int nr = r + dr[d];
            int nc = c + dc[d];
            if (!inBounds(nr, nc))                   continue;
            if (board[nr][nc] <= 0)                  continue;
            if (cellSet.count(make_pair(nr, nc)))    continue;
            usedColors.insert(colorBoard[nr][nc]);
        }
    }

    for (int col = 0; col < 3; col++)
        if (!usedColors.count(col))
            return col;

    return 0;
}

void solve(int tr, int tc, int size, int mr, int mc) {
    if (size == 2) {
        trominoID++;
        vector<pair<int,int> > cells;
        for (int r = tr; r < tr + 2; r++)
            for (int c = tc; c < tc + 2; c++)
                if (r != mr || c != mc)
                    cells.push_back(make_pair(r, c));

        int col = assignColor(cells);
        for (int i = 0; i < (int)cells.size(); i++) {
            board[cells[i].first][cells[i].second]      = trominoID;
            colorBoard[cells[i].first][cells[i].second] = col;
        }
        return;
    }

    int half = size / 2;
    int mq = (mr >= tr + half ? 2 : 0) + (mc >= tc + half ? 1 : 0);

    int ctr[4][2] = {
        {tr + half - 1, tc + half - 1},
        {tr + half - 1, tc + half    },
        {tr + half,     tc + half - 1},
        {tr + half,     tc + half    }
    };

    trominoID++;
    vector<pair<int,int> > centerCells;
    for (int q = 0; q < 4; q++)
        if (q != mq)
            centerCells.push_back(make_pair(ctr[q][0], ctr[q][1]));

    int col = assignColor(centerCells);
    for (int i = 0; i < (int)centerCells.size(); i++) {
        board[centerCells[i].first][centerCells[i].second]      = trominoID;
        colorBoard[centerCells[i].first][centerCells[i].second] = col;
    }

    int qo[4][2] = {
        {tr,        tc       },
        {tr,        tc + half},
        {tr + half, tc       },
        {tr + half, tc + half}
    };

    for (int q = 0; q < 4; q++) {
        int nmr = (q == mq) ? mr : ctr[q][0];
        int nmc = (q == mq) ? mc : ctr[q][1];
        solve(qo[q][0], qo[q][1], half, nmr, nmc);
    }
}

int main() {
    int n  = N;
    int mr = MISSING_R;
    int mc = MISSING_C;

    if (n <= 1) {
        printf("Error: N must be > 1 (minimum board is 4x4).\n");
        return 1;
    }

    int sz = 1 << n;
    BOARD_SIZE = sz;

    if (mr < 0 || mr >= sz || mc < 0 || mc >= sz) {
        printf("Error: missing cell (%d,%d) is outside the %dx%d board.\n",
               mr, mc, sz, sz);
        return 1;
    }

    printf("Divide and Conquer  |  n=%d, board=%dx%d, missing=(%d,%d)\n\n",
           n, sz, sz, mr, mc);

    memset(board,      0,  sizeof(board));
    memset(colorBoard, -1, sizeof(colorBoard));
    board[mr][mc] = -1;

    solve(0, 0, sz, mr, mc);

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
                if (!inBounds(nr, nc)) continue;
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

    return 0;
}


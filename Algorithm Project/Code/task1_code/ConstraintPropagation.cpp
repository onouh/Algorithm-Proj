#include <bits/stdc++.h>
using namespace std;

// -------------------------------------------------------
//  INPUT — edit these three lines to change the test case
//  N <= 4 is practical for CP (16x16)
// -------------------------------------------------------
const int N          = 4;   // board is 2^N x 2^N  (must be > 1)
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
int sz, trominoID = 0;

map<int, set<int> >               domains;
map<int, vector<pair<int,int> > > trominoCells;

set<int> getNeighborTrominoes(vector<pair<int,int> >& cells) {
    set<pair<int,int> > cellSet(cells.begin(), cells.end());
    set<int> nbrs;
    int dr[] = {0, 0,  1, -1};
    int dc[] = {1, -1, 0,  0};

    for (int i = 0; i < (int)cells.size(); i++) {
        int r = cells[i].first;
        int c = cells[i].second;
        for (int d = 0; d < 4; d++) {
            int nr = r + dr[d], nc = c + dc[d];
            if (nr < 0 || nr >= sz || nc < 0 || nc >= sz) continue;
            if (board[nr][nc] <= 0) continue;
            if (cellSet.count(make_pair(nr, nc))) continue;
            nbrs.insert(board[nr][nc]);
        }
    }
    return nbrs;
}

bool propagate(int tid, int col, map<int, set<int> >& dom) {
    queue<pair<int,int> > q;

    set<int> directNeighbors = getNeighborTrominoes(trominoCells[tid]);
    for (set<int>::iterator it = directNeighbors.begin();
         it != directNeighbors.end(); ++it)
        q.push(make_pair(*it, col));

    while (!q.empty()) {
        int t = q.front().first;
        int c = q.front().second;
        q.pop();

        if (!dom.count(t))    continue;
        if (!dom[t].count(c)) continue;

        dom[t].erase(c);

        if (dom[t].empty()) return false;

        if (dom[t].size() == 1) {
            int forced = *dom[t].begin();
            set<int> neighbors2 = getNeighborTrominoes(trominoCells[t]);
            for (set<int>::iterator it = neighbors2.begin();
                 it != neighbors2.end(); ++it) {
                int n2 = *it;
                if (!dom.count(n2))         continue;
                if (!dom[n2].count(forced)) continue;
                q.push(make_pair(n2, forced));
            }
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
            trominoCells[trominoID] = cells;

            // Initialize domain by removing colors of already-placed neighbors
            set<int> initDomain;
            initDomain.insert(0);
            initDomain.insert(1);
            initDomain.insert(2);
            {
                set<pair<int,int> > cellSet(cells.begin(), cells.end());
                int dr[] = {0, 0,  1, -1};
                int dc[] = {1, -1, 0,  0};
                for (int i = 0; i < (int)cells.size(); i++) {
                    int r2 = cells[i].first;
                    int c2 = cells[i].second;
                    for (int d = 0; d < 4; d++) {
                        int nr2 = r2 + dr[d], nc2 = c2 + dc[d];
                        if (nr2 < 0 || nr2 >= sz || nc2 < 0 || nc2 >= sz) continue;
                        if (board[nr2][nc2] <= 0) continue;
                        if (cellSet.count(make_pair(nr2, nc2))) continue;
                        if (colorBoard[nr2][nc2] >= 0)
                            initDomain.erase(colorBoard[nr2][nc2]);
                    }
                }
            }
            domains[trominoID] = initDomain;

            for (int col = 0; col < 3; col++) {
                if (!domains[trominoID].count(col)) continue;

                for (int i = 0; i < (int)cells.size(); i++) {
                    board[cells[i].first][cells[i].second]      = trominoID;
                    colorBoard[cells[i].first][cells[i].second] = col;
                }

                map<int, set<int> > savedDomains = domains;

                if (propagate(trominoID, col, domains)) {
                    if (solve(r, c)) return true;
                }

                domains = savedDomains;
                for (int i = 0; i < (int)cells.size(); i++) {
                    board[cells[i].first][cells[i].second]      = 0;
                    colorBoard[cells[i].first][cells[i].second] = -1;
                }
            }

            domains.erase(trominoID);
            trominoCells.erase(trominoID);
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
        printf("Error: N=%d gives a %dx%d board — too large for CP. Use N <= 4.\n",
               n, sz, sz);
        return 1;
    }

    if (mr < 0 || mr >= sz || mc < 0 || mc >= sz) {
        printf("Error: missing cell (%d,%d) is outside the %dx%d board.\n",
               mr, mc, sz, sz);
        return 1;
    }

    printf("Constraint Propagation  |  n=%d, board=%dx%d, missing=(%d,%d)\n\n",
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


// ============================================================
//  Task 1 — Tromino Tiling: Graph-Based Greedy Coloring

#include <bits/stdc++.h>
using namespace std;

// -------------------------------------------------------
//  All test cases run automatically.
//  To add a new case, add a runTest() call in main().
// -------------------------------------------------------

int board[512][512];
int colorBoard[512][512];
int BOARD_SIZE = 0;
int trominoID  = 0;

map<int, vector<pair<int,int> > > trominoCells; // tromino ID -> its 3 cells
map<int, set<int> >               adj;          // tromino adjacency graph

// -------------------------------------------------------
// Phase 1: Tile the board using D&C — no coloring yet
// -------------------------------------------------------
void tileOnly(int tr, int tc, int size, int mr, int mc) {
    if (size == 2) {
        trominoID++;
        for (int r = tr; r < tr + 2; r++)
            for (int c = tc; c < tc + 2; c++)
                if (r != mr || c != mc) {
                    board[r][c] = trominoID;
                    trominoCells[trominoID].push_back(make_pair(r, c));
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
    for (int q = 0; q < 4; q++)
        if (q != mq) {
            board[ctr[q][0]][ctr[q][1]] = trominoID;
            trominoCells[trominoID].push_back(make_pair(ctr[q][0], ctr[q][1]));
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
        tileOnly(qo[q][0], qo[q][1], half, nmr, nmc);
    }
}

// -------------------------------------------------------
// Phase 2: Build adjacency graph from placed trominoes
// -------------------------------------------------------
void buildGraph(int sz) {
    int dr[] = {0, 1};
    int dc[] = {1, 0};
    for (int r = 0; r < sz; r++)
        for (int c = 0; c < sz; c++) {
            if (board[r][c] <= 0) continue;
            int tid = board[r][c];
            for (int d = 0; d < 2; d++) {
                int nr = r + dr[d], nc = c + dc[d];
                if (nr < sz && nc < sz && board[nr][nc] > 0
                    && board[nr][nc] != tid) {
                    adj[tid].insert(board[nr][nc]);
                    adj[board[nr][nc]].insert(tid);
                }
            }
        }
}

// -------------------------------------------------------
// Phase 3: Greedy BFS 3-coloring on the adjacency graph
// -------------------------------------------------------
void greedyColor() {
    map<int,int> colorMap;
    if (trominoCells.empty()) return;

    queue<int> q;
    set<int>   visited;
    int start = trominoCells.begin()->first;
    q.push(start);
    visited.insert(start);

    while (!q.empty()) {
        int tid = q.front(); q.pop();

        // Collect colors used by already-colored neighbors
        set<int> used;
        for (set<int>::iterator it = adj[tid].begin();
             it != adj[tid].end(); ++it)
            if (colorMap.count(*it))
                used.insert(colorMap[*it]);

        // Assign first available color
        for (int col = 0; col < 3; col++)
            if (!used.count(col)) { colorMap[tid] = col; break; }

        // Enqueue unvisited neighbors
        for (set<int>::iterator it = adj[tid].begin();
             it != adj[tid].end(); ++it)
            if (!visited.count(*it)) {
                visited.insert(*it);
                q.push(*it);
            }
    }

    // Phase 4: Apply colorMap back to the board
    // FIX: replaced C++17 structured bindings with explicit iterator
    for (map<int, vector<pair<int,int> > >::iterator it = trominoCells.begin();
         it != trominoCells.end(); ++it) {
        int tid = it->first;
        vector<pair<int,int> >& cells = it->second;
        for (int i = 0; i < (int)cells.size(); i++)
            colorBoard[cells[i].first][cells[i].second] = colorMap[tid];
    }
}

// -------------------------------------------------------
// Run one complete test case
// -------------------------------------------------------
void runTest(int n, int mr, int mc) {
    if (n <= 1) { printf("Error: N must be > 1.\n"); return; }
    int sz = 1 << n;
    BOARD_SIZE = sz;
    if (mr < 0 || mr >= sz || mc < 0 || mc >= sz) {
        printf("Error: missing cell (%d,%d) out of bounds.\n", mr, mc);
        return;
    }

    // Reset all global state for this test
    trominoID = 0;
    trominoCells.clear();
    adj.clear();
    memset(board,      0,  sizeof(board));
    memset(colorBoard, -1, sizeof(colorBoard));
    board[mr][mc] = -1;

    printf("Graph-Based Coloring  |  n=%d  board=%dx%d  missing=(%d,%d)\n",
           n, sz, sz, mr, mc);

    tileOnly(0, 0, sz, mr, mc);   // Phase 1
    buildGraph(sz);                // Phase 2
    greedyColor();                 // Phase 3 + 4

    // Print board
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

    // Verify correctness
    bool valid = true;
    int dr[] = {0, 1};
    int dc[] = {1, 0};
    for (int r = 0; r < sz && valid; r++)
        for (int c = 0; c < sz && valid; c++) {
            if (board[r][c] <= 0) continue;
            for (int d = 0; d < 2; d++) {
                int nr = r + dr[d], nc = c + dc[d];
                if (nr >= sz || nc >= sz) continue;
                if (board[nr][nc] <= 0) continue;
                if (board[nr][nc] == board[r][c]) continue;
                if (colorBoard[nr][nc] == colorBoard[r][c]) {
                    valid = false;
                    printf("Color conflict at (%d,%d) and (%d,%d)!\n",
                           r, c, nr, nc);
                }
            }
        }
    printf("Color validity  : %s\n", valid ? "PASSED" : "FAILED");
    printf("Trominoes placed: %d\n\n", trominoID);
}

int main() {
   
    printf("  TROMINO TILING — GRAPH-BASED COLORING\n");

    // ---- 4x4 board (n=2): corner cases ----
    printf("--- 4x4 board: corner cases ---\n\n");
    runTest(2, 0, 0);   // top-left  corner missing
    runTest(2, 0, 3);   // top-right corner missing
    runTest(2, 3, 0);   // bottom-left corner missing
    runTest(2, 3, 3);   // bottom-right corner missing

    // ---- 4x4 board (n=2): interior cases ----
    printf("--- 4x4 board: interior cases ---\n\n");
    runTest(2, 1, 1);
    runTest(2, 2, 2);

    // ---- 8x8 board (n=3) ----
    printf("--- 8x8 board ---\n\n");
    runTest(3, 0, 0);
    runTest(3, 3, 4);

    return 0;
}

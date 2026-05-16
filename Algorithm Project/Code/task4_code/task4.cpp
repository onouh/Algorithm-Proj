/*
 * ============================================================
 *  Knight Swap Puzzle — 3x4 Chessboard
 *  Algorithm: Iterative Improvement (Hill Climbing)
 * ============================================================
 *  Board layout (cell indices):
 *    [ 0][ 1][ 2]   row 0  <- black start / white goal
 *    [ 3][ 4][ 5]   row 1
 *    [ 6][ 7][ 8]   row 2
 *    [ 9][10][11]   row 3  <- white start / black goal
 *
 *  W = White knights (start bottom row, goal top row)
 *  B = Black knights (start top row,   goal bottom row)
 * ============================================================
 */

#include <iostream>
#include <vector>
#include <array>
#include <unordered_set>
#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>
#include <limits>

using namespace std;

// Constants 
const int ROWS = 4;
const int COLS = 3;
const int CELLS = ROWS * COLS;  // 12

// Knight move offsets
const vector<pair<int, int>> KNIGHT_DELTAS = 
{
    {-2,-1},{-2,1},{-1,-2},{-1,2},
    { 1,-2},{ 1,2},{ 2,-1},{ 2, 1}
};

// Helpers
int toIndex(int r, int c)
{
    return r * COLS + c;
}

pair<int, int> toRC(int idx) 
{
    return { idx / COLS, idx % COLS };
}

// Pre-compute legal destinations for every cell
vector<vector<int>> buildKnightGraph()
{
    vector<vector<int>> g(CELLS);
    for (int i = 0; i < CELLS; i++)
    {
        int r = toRC(i).first;
        int c = toRC(i).second;
        for (int k = 0; k < (int)KNIGHT_DELTAS.size(); k++)
        {
            int nr = r + KNIGHT_DELTAS[k].first;
            int nc = c + KNIGHT_DELTAS[k].second;
            if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS)
                g[i].push_back(toIndex(nr, nc));
        }
    }
    return g;
}

const vector<vector<int>> GRAPH = buildKnightGraph();

// State 
// indices 0-2 = white knights, indices 3-5 = black knights
typedef array<int, 6> State;

const State START_STATE = {
    toIndex(3,0), toIndex(3,1), toIndex(3,2),   // whites at bottom
    toIndex(0,0), toIndex(0,1), toIndex(0,2)    // blacks at top
};

// Canonical key (sorted within each color group)
string stateKey(const State& s) {
    array<int, 3> w = { s[0], s[1], s[2] };
    array<int, 3> b = { s[3], s[4], s[5] };
    sort(w.begin(), w.end());
    sort(b.begin(), b.end());
    ostringstream oss;
    for (int i = 0; i < 3; i++) oss << w[i] << ',';
    oss << '|';
    for (int i = 0; i < 3; i++) oss << b[i] << ',';
    return oss.str();
}

bool isGoal(const State& s) {
    array<int, 3> w = { s[0], s[1], s[2] };
    array<int, 3> b = { s[3], s[4], s[5] };
    sort(w.begin(), w.end());
    sort(b.begin(), b.end());
    array<int, 3> gw = { toIndex(0,0), toIndex(0,1), toIndex(0,2) };
    array<int, 3> gb = { toIndex(3,0), toIndex(3,1), toIndex(3,2) };
    return w == gw && b == gb;
}

// Move struct
struct Move {
    int  pieceIdx;   // 0-2 white, 3-5 black
    int  from;
    int  to;
    char color;      // 'W' or 'B'
};

// Generate all legal successor (state, move) pairs
vector<pair<State, Move>> successors(const State& s) {
    unordered_set<int> occupied(s.begin(), s.end());
    vector<pair<State, Move>> result;

    for (int i = 0; i < 6; i++) {
        char color = (i < 3) ? 'W' : 'B';
        for (int j = 0; j < (int)GRAPH[s[i]].size(); j++) {
            int dest = GRAPH[s[i]][j];
            if (occupied.count(dest)) continue;
            State ns = s;
            ns[i] = dest;
            Move mv;
            mv.pieceIdx = i;
            mv.from = s[i];
            mv.to = dest;
            mv.color = color;
            result.push_back({ ns, mv });
        }
    }
    return result;
}

// Display 
void printBoard(const State& s) {
    array<char, 12> grid;
    grid.fill('.');
    for (int i = 0; i < 3; i++) grid[s[i]] = 'W';
    for (int i = 3; i < 6; i++) grid[s[i]] = 'B';

    string hline = "  +---+---+---+";
    cout << hline << "\n";
    for (int r = 0; r < ROWS; r++) {
        cout << r << " |";
        for (int c = 0; c < COLS; c++)
            cout << " " << grid[toIndex(r, c)] << " |";
        cout << "\n" << hline << "\n";
    }
    cout << "    a   b   c\n\n";
}

void printMove(int step, const Move& m)
{
    int fr = toRC(m.from).first;
    int fc = toRC(m.from).second;
    int tr = toRC(m.to).first;
    int tc = toRC(m.to).second;
    char cols[] = { 'a', 'b', 'c' };
    cout << "  Step " << setw(2) << step << ": "
        << (m.color == 'W' ? "White" : "Black")
        << " knight  "
        << cols[fc] << (ROWS - fr)
        << "  ->  "
        << cols[tc] << (ROWS - tr)
        << "\n";
}

// ════════════════════════════════════════════════════════════
//  HEURISTIC
//  h(state) = sum of row-distances each piece must still travel
//    White knights want to reach row 0  -> cost = current row
//    Black knights want to reach row 3  -> cost = 3 - current row
//  Lower is better. h=0 means goal reached.
// ════════════════════════════════════════════════════════════

int heuristic(const State& s) 
{
    int cost = 0;
    for (int i = 0; i < 3; i++)
        cost += toRC(s[i]).first;                // white: wants row 0
    for (int i = 3; i < 6; i++)
        cost += (ROWS - 1 - toRC(s[i]).first);  // black: wants row 3
    return cost;
}

// ════════════════════════════════════════════════════════════
//  ITERATIVE IMPROVEMENT (Hill Climbing)
//
//  At every step:
//    1. Evaluate h for all legal next states.
//    2. Move to the state with the LOWEST h (greedy best).
//    3. If no unvisited state improves (or matches) h -> STUCK.
//
//  Limitation: can get permanently stuck in a local minimum
//  where every move increases h, even though the goal requires
//  temporarily moving pieces away from their targets.
// ════════════════════════════════════════════════════════════

void iterativeImprovement()
{
    State cur = START_STATE;
    vector<Move> path;
    unordered_set<string> visited;
    visited.insert(stateKey(cur));

    const int MAX_STEPS = 1000;

    cout << "Iterative Improvement\n\n";
    cout << "Heuristic h(start) = " << heuristic(cur) << "\n\n";
    cout << "Start position:\n";
    printBoard(cur);

    for (int step = 1; step <= MAX_STEPS; step++) {

        if (isGoal(cur)) 
        {
            cout << "============================================\n";
            cout << "  GOAL REACHED in " << path.size() << " moves!\n";
            cout << "============================================\n\n";
            return;
        }

        int curH = heuristic(cur);
        int bestH = numeric_limits<int>::max();
        State bestState = cur;
        Move  bestMove;
        bool  found = false;

        // find best improving move 
        vector<pair<State, Move>> succ = successors(cur);
        for (int i = 0; i < (int)succ.size(); i++) 
        {
            State& ns = succ[i].first;
            Move& mv = succ[i].second;
            string k = stateKey(ns);
            if (visited.count(k)) continue;
            int h = heuristic(ns);
            if (h < bestH) 
            {
                bestH = h;
                bestState = ns;
                bestMove = mv;
                found = true;
            }
        }

        // --- if no strictly improving move, allow sideways (same h) ---
        if (!found || bestH > curH)
        {
            found = false;
            for (int i = 0; i < (int)succ.size(); i++) 
            {
                State& ns = succ[i].first;
                Move& mv = succ[i].second;
                string k = stateKey(ns);
                if (visited.count(k)) continue;
                int h = heuristic(ns);
                if (h <= curH) 
                {
                    bestState = ns;
                    bestMove = mv;
                    found = true;
                    break;
                }
            }
        }

        // --- truly stuck: local minimum ---
        if (!found) 
        {
            cout << "--------------------------------------------\n";
            cout << "  STUCK at step " << step - 1
                << "  (h = " << curH << ")\n";
            cout << "  Every unvisited move increases the cost.\n";
            cout << "  Iterative Improvement FAILED to find goal.\n";
            cout << "--------------------------------------------\n";
            return;
        }

        // --- apply best move ---
        printMove(step, bestMove);
        cout << "  h = " << curH << "  ->  " << bestH << "\n";
        visited.insert(stateKey(bestState));
        cur = bestState;
        path.push_back(bestMove);
        printBoard(cur);
    }

    cout << "Exceeded maximum steps (" << MAX_STEPS << "). No solution.\n";
}

int main() 
{
    iterativeImprovement();
    return 0;
}
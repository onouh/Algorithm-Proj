#include <iostream>
#include <vector>
#include <cmath>
using namespace std;

int n;
int dx[8] = {2, 1, -1, -2, -2, -1, 1, 2};
int dy[8] = {1, 2, 2, 1, -1, -2, -2, -1};

bool isValid(int x, int y, vector<vector<int>>& board) {
    return (x >= 0 && y >= 0 && x < n && y < n && board[x][y] == -1);
}
int getDegree(int x, int y, vector<vector<int>>& board) {
    int count = 0;
    for (int i = 0; i < 8; i++) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (isValid(nx, ny, board))
            count++;
    }
    return count;
}
int nextMoveScore(int x, int y, vector<vector<int>>& board) {
    int score = 0;
    for (int i = 0; i < 8; i++) {
        int nx = x + dx[i];



        int ny = y + dy[i];
        if (isValid(nx, ny, board))
            score += getDegree(nx, ny, board);
    }
    return score;
}
double distFromCenter(int x, int y) {
    double cx = (n - 1) / 2.0;
    double cy = (n - 1) / 2.0;
    return sqrt((x - cx)*(x - cx) + (y - cy)*(y - cy));
}
bool solve(int startX, int startY, vector<vector<int>>& board) {
    board[startX][startY] = 0;
    int currX = startX, currY = startY;
    for (int step = 1; step < n * n; step++) {
        int nextX = -1, nextY = -1;
        int minDegree = INT_MAX;
        int bestScore = INT_MAX;
        for (int i = 0; i < 8; i++) {
            int nx = currX + dx[i];
            int ny = currY + dy[i];
            if (!isValid(nx, ny, board)) continue;
            int deg = getDegree(nx, ny, board);
            if (deg < minDegree) {
                minDegree = deg;
                bestScore = nextMoveScore(nx, ny, board);
                nextX = nx;
                nextY = ny;
            }
            else if (deg == minDegree) {
                int score = nextMoveScore(nx, ny, board);
                if (score < bestScore) {
                    bestScore = score;
                    nextX = nx;

                    nextY = ny;
                }
                else if (score == bestScore) {
                    if (distFromCenter(nx, ny) < distFromCenter(nextX, nextY)) {
                        nextX = nx;
                        nextY = ny;
                    }
                }
            }
        }
        if (nextX == -1)
            return false;
        board[nextX][nextY] = step;
        currX = nextX;
        currY = nextY;
    }
    for (int i = 0; i < 8; i++) {
        if (currX + dx[i] == startX && currY + dy[i] == startY)
            return true;
    }
    return false;
}
int main() {
    cout << "Enter board size (n): ";
    cin >> n;
    if (n < 6) {
        cout << "Closed knight tour not possible for n < 6.\n";
        return 0;
    }
    if(n>6 && n%2==1){
        cout << "Closed knight tour not possible for odd n > 6.\n";
        return 0;
    }
    vector<vector<int>> board;

    // Try all possible starting points
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {

            board.assign(n, vector<int>(n, -1));

            if (solve(i, j, board)) {
                cout << "\nClosed Knight Tour Found:\n\n";

                for (int x = 0; x < n; x++) {
                    for (int y = 0; y < n; y++)
                        cout << board[x][y] << "\t";
                    cout << endl;
                }

                return 0;
            }
        }
    }
    cout << "No closed tour found.\n";
    return 0;
}

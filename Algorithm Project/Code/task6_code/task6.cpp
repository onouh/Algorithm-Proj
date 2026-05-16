#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

// Represents an (x, y) coordinate on our lattice
struct Point {
    int x, y;
};

// Generates the sequence of vertices to connect for an n x n grid
vector<Point> solveDots(int n) {
    vector<Point> path;
    if (n < 3) return path; // The classic puzzle requires n >= 3

    // Boundaries of the currently unsolved grid dots
    int x0 = 0, x1 = n - 1;
    int y0 = 0, y1 = n - 1;

    // Directions -> 0: Right, 1: Down, 2: Left, 3: Up
    int dir = 0; 
    
    // Start at the top-left corner
    Point current = {x0, y1};
    path.push_back(current);

    // Phase 1: Spiral inward (decreasing lattice) until a 3x3 grid remains
    while ((x1 - x0) > 2 || (y1 - y0) > 2) {
        if (dir == 0) {
            current = {x1, current.y};
            y1--; // Top row solved
        } else if (dir == 1) {
            current = {current.x, y0};
            x1--; // Right col solved
        } else if (dir == 2) {
            current = {x0, current.y};
            y0++; // Bottom row solved
        } else if (dir == 3) {
            current = {current.x, y1};
            x0++; // Left col solved
        }
        path.push_back(current);
        dir = (dir + 1) % 4; // Turn 90 degrees clockwise
    }

    // Phase 2: Solve the remaining 3x3 core
    // At this point, the box [x0, x1] x [y0, y1] is exactly a 3x3 grid.
    // We attach the classic 4-line 9-dot solution rotated to our current heading.
    Point p1, p2, p3, p4;

    if (dir == 0) {
        // Approaching from the Left, facing Right
        p1 = {x0 + 3, y0 + 2};
        p2 = {x0 + 0, y0 - 1};
        p3 = {x0 + 0, y0 + 2};
        p4 = {x0 + 2, y0 + 0};
    } else if (dir == 1) {
        // Approaching from the Top, facing Down
        p1 = {x0 + 2, y0 - 1};
        p2 = {x0 - 1, y0 + 2};
        p3 = {x0 + 2, y0 + 2};
        p4 = {x0 + 0, y0 + 0};
    } else if (dir == 2) {
        // Approaching from the Right, facing Left
        p1 = {x0 - 1, y0 + 0};
        p2 = {x0 + 2, y0 + 3};
        p3 = {x0 + 2, y0 + 0};
        p4 = {x0 + 0, y0 + 2};
    } else if (dir == 3) {
        // Approaching from the Bottom, facing Up
        p1 = {x0 + 0, y0 + 3};
        p2 = {x0 + 3, y0 + 0};
        p3 = {x0 + 0, y0 + 0};
        p4 = {x0 + 2, y0 + 2};
    }

    path.push_back(p1);
    path.push_back(p2);
    path.push_back(p3);
    path.push_back(p4);

    return path;
}

// Verification Helper: Marks points along a straight line segment as visited
void markVisited(Point p1, Point p2, vector<vector<bool>>& visited, int n) {
    int dx = p2.x - p1.x;
    int dy = p2.y - p1.y;
    int steps = max(abs(dx), abs(dy));
    
    for(int i = 0; i <= steps; ++i) {
        int x = p1.x + (steps == 0 ? 0 : dx * i / steps);
        int y = p1.y + (steps == 0 ? 0 : dy * i / steps);
        
        // Only mark if the point falls inside our actual real n*n grid bounds
        if(x >= 0 && x < n && y >= 0 && y < n) {
            visited[x][y] = true;
        }
    }
}

int main() {
    int n;
    cout << "Enter lattice size n (n >= 3): ";
    if (!(cin >> n) || n < 3) {
        cout << "Error: n must be >= 3." << endl;
        return 1;
    }

    vector<Point> solution = solveDots(n);

    cout << "\n===========================================\n";
    cout << " Optimal Continuous Path for " << n << "x" << n << " grid\n";
    cout << "===========================================\n";
    for (size_t i = 0; i < solution.size(); ++i) {
        cout << " Vertex " << i << ": (" << solution[i].x << ", " << solution[i].y << ")";
        if (solution[i].x < 0 || solution[i].x >= n || solution[i].y < 0 || solution[i].y >= n) {
            cout << "  <-- (Extends outside grid to angle the pen)";
        }
        cout << "\n";
    }
    
    // The number of straight lines drawn is the number of vertices minus 1
    int total_lines = solution.size() - 1;
    cout << "-------------------------------------------\n";
    cout << "Total straight lines used: " << total_lines 
         << " (Mathematical minimum: " << 2 * n - 2 << ")\n";

    // ------------------------------------------------------------
    // Verification Phase (Adapted from v2 & v3 logic)
    // ------------------------------------------------------------
    vector<vector<bool>> visited(n, vector<bool>(n, false));
    
    for (size_t i = 0; i < solution.size() - 1; ++i) {
        markVisited(solution[i], solution[i+1], visited, n);
    }

    cout << "\nLattice Coverage Verification:\n";
    int count = 0;
    for (int y = n - 1; y >= 0; --y) {
        cout << "  y=" << y << " |\t";
        for (int x = 0; x < n; ++x) {
            if (visited[x][y]) {
                cout << "[X] ";
                count++;
            } else {
                cout << " .  ";
            }
        }
        cout << "\n";
    }
    cout << "\nTotal real points covered: " << count << " / " << n * n << "\n";
    if (count == n * n) {
        cout << "SUCCESS! All dots connected without lifting the pen.\n";
    }

    return 0;
}
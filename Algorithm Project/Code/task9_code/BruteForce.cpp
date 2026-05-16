#include <iostream>
#include <vector>
#include <cmath>
#include <limits>
#include <iomanip>
#include <string>
#include <set>
#include <numeric>
#include <algorithm>
#include <functional>

// ─────────────────────────────────────────────────────────────────
// brute force k-clustering
// time: o(k^n * n * k) | space: o(n) | guarantees global optimum
// only practical for n <= ~15, k <= 3
// ─────────────────────────────────────────────────────────────────

// terminal styling
const std::string RESET   = "\033[0m";
const std::string BOLD    = "\033[1m";
const std::string DIM     = "\033[2m";
const std::string COLORS[]= {"\033[91m","\033[92m","\033[93m","\033[94m","\033[95m"};
const char        SYMBOLS[]= {'@','#','%','&','*'};

using Point     = std::pair<double,double>;
using Cluster   = std::vector<Point>;
using Partition = std::vector<Cluster>;


// ── stirling number of the second kind ──────────────────────────
// s(n,k) = (1/k!) * sum_{j=0}^{k} (-1)^(k-j) * C(k,j) * j^n
// exact formula as given in section 2.1 of the research
long long stirling(int n, int k) {
    // binomial coefficient c(k, j)
    auto binom = [](int a, int b) -> long long {
        if (b > a || b < 0) return 0;
        long long r = 1;
        for (int i = 0; i < b; i++) r = r * (a - i) / (i + 1);
        return r;
    };

    double sum = 0.0;
    for (int j = 0; j <= k; j++) {
        double sign = ((k - j) % 2 == 0) ? 1.0 : -1.0;
        sum += sign * (double)binom(k, j) * std::pow((double)j, (double)n);
    }

    // divide by k!
    long long kfact = 1;
    for (int i = 2; i <= k; i++) kfact *= i;

    return (long long)std::round(sum / (double)kfact);
}


// ── math helpers ─────────────────────────────────────────────────

Point compute_centroid(const Cluster& c) {
    double sx = 0, sy = 0;
    for (auto& p : c) { sx += p.first; sy += p.second; }
    return {sx / c.size(), sy / c.size()};
}

double compute_sse(const Partition& partition) {
    double total = 0.0;
    for (auto& cluster : partition) {
        auto [cx, cy] = compute_centroid(cluster);
        for (auto& p : cluster) {
            double dx = p.first  - cx;
            double dy = p.second - cy;
            total += dx*dx + dy*dy;
        }
    }
    return total;
}

double cluster_sse(const Cluster& c) {
    auto [cx, cy] = compute_centroid(c);
    double t = 0;
    for (auto& p : c) {
        double dx = p.first - cx, dy = p.second - cy;
        t += dx*dx + dy*dy;
    }
    return t;
}


// ── partition generator ──────────────────────────────────────────
// iterates all k^n label assignments; keeps only surjective ones
// (all k labels must appear → all clusters non-empty)

void enumerate_partitions(
    const std::vector<Point>& points,
    int k,
    std::function<void(const std::vector<int>&)> callback)
{
    int n = (int)points.size();
    std::vector<int> assignment(n, 0);

    // iterate all k^n combinations
    while (true) {
        // check surjectivity: all k labels used
        std::set<int> used(assignment.begin(), assignment.end());
        if ((int)used.size() == k) {
            callback(assignment);
        }

        // increment assignment like a base-k counter
        int pos = n - 1;
        while (pos >= 0 && assignment[pos] == k - 1) {
            assignment[pos--] = 0;
        }
        if (pos < 0) break;
        assignment[pos]++;
    }
}


// ── main algorithm ────────────────────────────────────────────────

struct Result {
    Partition   partition;
    std::vector<int> assignment;
    double      cost;
    long long   evaluated;
};

Result brute_force_clustering(const std::vector<Point>& points, int k) {
    int n = (int)points.size();
    double    best_cost = std::numeric_limits<double>::infinity();
    std::vector<int> best_assign(n, 0);
    long long count = 0;

    enumerate_partitions(points, k, [&](const std::vector<int>& asgn) {
        count++;

        // build partition from this assignment
        Partition part(k);
        for (int i = 0; i < n; i++)
            part[asgn[i]].push_back(points[i]);

        double cost = compute_sse(part);
        if (cost < best_cost) {
            best_cost  = cost;
            best_assign = asgn;
        }
    });

    // reconstruct best partition
    Partition best(k);
    for (int i = 0; i < n; i++)
        best[best_assign[i]].push_back(points[i]);

    return {best, best_assign, best_cost, count};
}


// ── ascii scatter plot ────────────────────────────────────────────

void ascii_scatter(
    const std::vector<Point>& points,
    const std::vector<int>&   assignment,
    int k,
    int width  = 62,
    int height = 20)
{
    double xmin = 1e9, xmax = -1e9, ymin = 1e9, ymax = -1e9;
    for (auto& p : points) {
        xmin = std::min(xmin, p.first);  xmax = std::max(xmax, p.first);
        ymin = std::min(ymin, p.second); ymax = std::max(ymax, p.second);
    }
    double xr = xmax - xmin == 0 ? 1 : xmax - xmin;
    double yr = ymax - ymin == 0 ? 1 : ymax - ymin;

    // build grid: store color+symbol string per cell
    std::vector<std::vector<std::string>> grid(height, std::vector<std::string>(width, " "));

    for (int i = 0; i < (int)points.size(); i++) {
        int col = (int)((points[i].first  - xmin) / xr * (width  - 1));
        int row = (int)((points[i].second - ymin) / yr * (height - 1));
        row = (height - 1) - row;   // flip y-axis
        col = std::clamp(col, 0, width  - 1);
        row = std::clamp(row, 0, height - 1);
        int lbl = assignment[i];
        grid[row][col] = COLORS[lbl % 5] + SYMBOLS[lbl % 5] + RESET;
    }

    // draw
    std::string top    = "  " + DIM + "╔" + std::string(width + 2, '═') + "╗" + RESET;
    std::string bottom = "  " + DIM + "╚" + std::string(width + 2, '═') + "╝" + RESET;

    std::cout << "\n" << top << "\n";
    for (auto& row : grid) {
        std::cout << "  " << DIM << "║" << RESET << " ";
        for (auto& cell : row) std::cout << cell;
        std::cout << " " << DIM << "║" << RESET << "\n";
    }
    std::cout << bottom << "\n";
    std::cout << "        " << DIM
              << std::fixed << std::setprecision(1) << xmin
              << std::string(width - 10, ' ')
              << xmax << RESET << "\n";
}


// ── cluster summary table ─────────────────────────────────────────

void print_summary(const Partition& partition, int k) {
    std::cout << "\n  " << std::string(56, '-') << "\n";
    std::cout << "  " << std::left
              << std::setw(10) << "cluster"
              << std::setw(8)  << "symbol"
              << std::setw(8)  << "size"
              << std::setw(20) << "centroid"
              << "sse\n";
    std::cout << "  " << std::string(56, '-') << "\n";

    for (int i = 0; i < k; i++) {
        auto [cx, cy] = compute_centroid(partition[i]);
        double sse    = cluster_sse(partition[i]);
        std::cout << "  C" << std::left << std::setw(9) << i+1
                  << COLORS[i % 5] << SYMBOLS[i % 5] << RESET
                  << std::setw(7) << ""
                  << std::setw(8) << partition[i].size()
                  << "(" << std::fixed << std::setprecision(2)
                  << std::setw(5) << cx << ", "
                  << std::setw(5) << cy << ")"
                  << std::setw(10) << ""
                  << std::setprecision(4) << sse << "\n";
    }
    std::cout << "  " << std::string(56, '-') << "\n";
}


// ── entry point ───────────────────────────────────────────────────

int main() {
    // sample dataset — small enough for brute force (n=12, k=3)
    std::vector<Point> points = {
        {1.0, 1.2}, {1.5, 0.8}, {1.2, 1.5}, {0.8, 1.0},   // cluster a (bottom-left)
        {5.0, 5.3}, {5.5, 4.8}, {4.8, 5.1}, {5.2, 5.6},   // cluster b (top-right)
        {1.0, 5.0}, {1.3, 4.7}, {0.7, 5.2}, {1.1, 5.5},   // cluster c (top-left)
    };
    int k = 3;
    int n = (int)points.size();

    long long s = stirling(n, k);

    // ── header ────────────────────────────────────────────────────
    std::cout << "\n"
              << "  " << BOLD << "╔══════════════════════════════════════════════════════╗\n"
              << "  ║      BRUTE FORCE K-CLUSTERING  [ n=" << n << ", k=" << k << " ]        ║\n"
              << "  ╚══════════════════════════════════════════════════════╝" << RESET << "\n\n"
              << "  " << DIM << "algorithm  : enumerate all s(n,k) partitions\n"
              << "  complexity : o(k^n · n · k)\n"
              << "  guarantee  : global optimum ✔" << RESET << "\n\n"
              << "  points     → " << n << "\n"
              << "  clusters   → " << k << "\n"
              << "  s(" << n << "," << k << ")     → " << s << "  (exact stirling formula)\n";

    std::cout << "\n  " << DIM << "searching all partitions ..." << RESET << std::flush;

    Result r = brute_force_clustering(points, k);

    std::cout << " done  (" << r.evaluated << " evaluated)\n";

    // ── results ───────────────────────────────────────────────────
    std::cout << "\n  " << BOLD << "▸ OPTIMAL PARTITION FOUND" << RESET << "\n"
              << "  total sse  : " << BOLD << std::fixed << std::setprecision(6)
              << r.cost << RESET << "\n"
              << "  evaluated  : " << r.evaluated << " partitions\n";

    // ── legend ────────────────────────────────────────────────────
    std::cout << "\n  legend:\n";
    for (int i = 0; i < k; i++)
        std::cout << "    " << COLORS[i%5] << SYMBOLS[i%5] << RESET
                  << "  →  cluster " << i+1 << "\n";

    // ── scatter plot ──────────────────────────────────────────────
    ascii_scatter(points, r.assignment, k);

    // ── cluster table ─────────────────────────────────────────────
    print_summary(r.partition, k);

    // ── complexity note ───────────────────────────────────────────
    std::cout << "\n  " << DIM
              << "⚠  practical limit: n ≤ 15, k ≤ 3\n"
              << "   s(20,5) ≈ 4.3 billion  →  borderline feasible\n"
              << "   s(50,5) ≈ 10^32        →  completely infeasible" << RESET << "\n\n";

    return 0;
}
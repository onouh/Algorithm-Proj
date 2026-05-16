#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
#include <cmath>
#include <limits>
#include <algorithm>
#include <numeric>
#include <string>
#include <random>
#include <cassert>

// ─────────────────────────────────────────────────────────────────
// iterative improvement — three algorithms from section 4:
//   1. lloyd's k-means         o(n·k·i·d)
//   2. k-means++ initialisation  o(n·k)
//   3. k-medoids (pam)         o(n²·k)
// ─────────────────────────────────────────────────────────────────

static const std::string RESET    = "\033[0m";
static const std::string BOLD     = "\033[1m";
static const std::string DIM      = "\033[2m";
static const std::string GREEN    = "\033[92m";
static const std::string YELLOW   = "\033[93m";
static const std::string RED      = "\033[91m";
static const std::string CYAN     = "\033[96m";
static const std::string COLORS[] = { "\033[91m","\033[92m","\033[93m",
                                       "\033[94m","\033[95m","\033[96m" };
static const char        SYMBOLS[] = { '@','#','%','&','*','~' };
static const int         NS = 6;

using Point  = std::array<double, 2>;

std::mt19937 rng(42);   // fixed seed for reproducibility


// ── geometry helpers ─────────────────────────────────────────────

double dist_sq(const Point& a, const Point& b) {
    double dx = a[0]-b[0], dy = a[1]-b[1];
    return dx*dx + dy*dy;
}

Point centroid(const std::vector<Point>& pts) {
    Point mu = {0.0, 0.0};
    for (const auto& p : pts) { mu[0]+=p[0]; mu[1]+=p[1]; }
    mu[0] /= pts.size(); mu[1] /= pts.size();
    return mu;
}

double compute_sse(const std::vector<Point>& pts,
                   const std::vector<Point>& centroids,
                   const std::vector<int>&   labels) {
    double sse = 0.0;
    for (int i=0;i<(int)pts.size();++i)
        sse += dist_sq(pts[i], centroids[labels[i]]);
    return sse;
}

double compute_sse_medoids(const std::vector<Point>& pts,
                           const std::vector<Point>& medoids,
                           const std::vector<int>&   labels) {
    return compute_sse(pts, medoids, labels);
}


// ─────────────────────────────────────────────────────────────────
// algorithm 1: k-means++ initialisation  (section 4.3)
// guarantee: o(log k) approximation ratio in expectation
// ─────────────────────────────────────────────────────────────────

std::vector<Point> kmeans_pp_init(const std::vector<Point>& pts, int k) {
    int n = (int)pts.size();
    std::vector<Point> mu;
    mu.reserve(k);

    // line 1: choose first centroid uniformly at random
    std::uniform_int_distribution<int> uniform(0, n-1);
    mu.push_back(pts[uniform(rng)]);

    std::vector<double> D(n);

    for (int j=1; j<k; ++j) {
        // lines 3-5: D(p) = min distance² to any already-chosen centroid
        double total = 0.0;
        for (int i=0; i<n; ++i) {
            double min_d = std::numeric_limits<double>::infinity();
            for (const auto& m : mu)
                min_d = std::min(min_d, dist_sq(pts[i], m));
            D[i]   = min_d;
            total += min_d;
        }
        // line 7: sample next centroid with probability ∝ D(p)
        std::uniform_real_distribution<double> pick(0.0, total);
        double r = pick(rng);
        double cum = 0.0;
        for (int i=0; i<n; ++i) {
            cum += D[i];
            if (cum >= r) { mu.push_back(pts[i]); break; }
        }
        // edge-guard: if nothing was picked, add last point
        if ((int)mu.size() < j+1) mu.push_back(pts[n-1]);
    }
    return mu;
}


// ─────────────────────────────────────────────────────────────────
// algorithm 2: lloyd's k-means  (section 4.2)
// ─────────────────────────────────────────────────────────────────

struct KMeansResult {
    std::vector<int>   labels;      // cluster assignment per point
    std::vector<Point> centroids;
    double             sse;
    int                iterations;
    std::vector<double> sse_history; // sse after each iteration
};

KMeansResult kmeans(const std::vector<Point>& pts,
                    int k,
                    int max_iter = 300,
                    bool use_pp  = true) {
    int n = (int)pts.size();

    // initialisation — line 2: random or k-means++
    std::vector<Point> mu = use_pp
        ? kmeans_pp_init(pts, k)
        : [&]() {
              std::vector<int> idx(n);
              std::iota(idx.begin(), idx.end(), 0);
              std::shuffle(idx.begin(), idx.end(), rng);
              std::vector<Point> m(k);
              for (int i=0;i<k;++i) m[i] = pts[idx[i]];
              return m;
          }();

    std::vector<int> C(n, 0), C_prev(n, -1);
    std::vector<double> sse_history;
    int iter = 0;

    // line 6: while assignments changed and under max_iter
    while (C != C_prev && iter < max_iter) {
        C_prev = C;

        // ── assignment step (lines 10-13) ─────────────────────────
        for (int i=0;i<n;++i) {
            int    best   = 0;
            double best_d = dist_sq(pts[i], mu[0]);
            for (int j=1;j<k;++j) {
                double d = dist_sq(pts[i], mu[j]);
                if (d < best_d) { best_d=d; best=j; }
            }
            C[i] = best;
        }

        // ── update step (lines 15-22) ─────────────────────────────
        for (int j=0;j<k;++j) {
            std::vector<Point> members;
            for (int i=0;i<n;++i)
                if (C[i]==j) members.push_back(pts[i]);

            if (!members.empty()) {
                mu[j] = centroid(members);
            } else {
                // line 20: re-init empty cluster with a random point
                std::uniform_int_distribution<int> u(0,n-1);
                mu[j] = pts[u(rng)];
            }
        }

        double sse = compute_sse(pts, mu, C);
        sse_history.push_back(sse);
        ++iter;
    }

    return { C, mu, compute_sse(pts, mu, C), iter, sse_history };
}


// ─────────────────────────────────────────────────────────────────
// algorithm 3: k-medoids / pam  (section 4.4)
// representatives must be actual data points — robust to outliers
// ─────────────────────────────────────────────────────────────────

struct PAMResult {
    std::vector<int>   labels;
    std::vector<Point> medoids;
    double             sse;
    int                swaps;
};

std::vector<int> assign_to_medoids(const std::vector<Point>& pts,
                                    const std::vector<Point>& medoids) {
    std::vector<int> labels(pts.size());
    for (int i=0;i<(int)pts.size();++i) {
        int    best   = 0;
        double best_d = dist_sq(pts[i], medoids[0]);
        for (int j=1;j<(int)medoids.size();++j) {
            double d = dist_sq(pts[i], medoids[j]);
            if (d < best_d) { best_d=d; best=j; }
        }
        labels[i] = best;
    }
    return labels;
}

double sse_with_medoids(const std::vector<Point>& pts,
                         const std::vector<Point>& medoids) {
    auto labels = assign_to_medoids(pts, medoids);
    double sse = 0.0;
    for (int i=0;i<(int)pts.size();++i)
        sse += dist_sq(pts[i], medoids[labels[i]]);
    return sse;
}

PAMResult pam_kmedoids(const std::vector<Point>& pts, int k) {
    int n = (int)pts.size();

    // line 1: pick k random initial medoids from actual data points
    std::vector<int> idx(n);
    std::iota(idx.begin(), idx.end(), 0);
    std::shuffle(idx.begin(), idx.end(), rng);

    std::vector<Point> M(k);
    for (int i=0;i<k;++i) M[i] = pts[idx[i]];

    double current_cost = sse_with_medoids(pts, M);
    bool   improved     = true;
    int    swaps        = 0;

    // repeat until no improving swap is found
    while (improved) {
        improved = false;

        // lines 9-10: try swapping each medoid m with each non-medoid o
        for (int mi=0; mi<k && !improved; ++mi) {
            for (int oi=0; oi<n && !improved; ++oi) {
                // skip if pts[oi] is already a medoid
                bool is_medoid = false;
                for (const auto& m : M)
                    if (m == pts[oi]) { is_medoid=true; break; }
                if (is_medoid) continue;

                // line 11: compute cost of swapping M[mi] with pts[oi]
                std::vector<Point> M_trial = M;
                M_trial[mi] = pts[oi];
                double trial_cost = sse_with_medoids(pts, M_trial);

                // lines 12-15: accept if improvement found
                if (trial_cost < current_cost) {
                    M            = M_trial;
                    current_cost = trial_cost;
                    improved     = true;
                    ++swaps;
                }
            }
        }
    }

    auto labels = assign_to_medoids(pts, M);
    return { labels, M, current_cost, swaps };
}


// ── ascii scatter plot ───────────────────────────────────────────

void ascii_scatter(const std::vector<Point>& pts,
                   const std::vector<int>&   labels,
                   const std::vector<Point>* special = nullptr, // medoids/centroids
                   int width=62, int height=20) {
    double xmin=1e9,xmax=-1e9,ymin=1e9,ymax=-1e9;
    for (const auto& p : pts) {
        xmin=std::min(xmin,p[0]); xmax=std::max(xmax,p[0]);
        ymin=std::min(ymin,p[1]); ymax=std::max(ymax,p[1]);
    }
    double xr = xmax-xmin>0 ? xmax-xmin : 1.0;
    double yr = ymax-ymin>0 ? ymax-ymin : 1.0;

    std::vector<std::vector<std::string>> grid(height,
        std::vector<std::string>(width," "));

    // place cluster members
    for (int i=0;i<(int)pts.size();++i) {
        int col=(int)((pts[i][0]-xmin)/xr*(width -1));
        int row=(int)((pts[i][1]-ymin)/yr*(height-1));
        row=(height-1)-row;
        col=std::clamp(col,0,width-1);
        row=std::clamp(row,0,height-1);
        int lbl=labels[i];
        std::string cell;
        cell += COLORS[lbl%NS];
        cell += SYMBOLS[lbl%NS];
        cell += RESET;
        grid[row][col] = cell;
    }

    // overlay centroids/medoids with a bright '✦' marker
    if (special) {
        for (int i=0;i<(int)special->size();++i) {
            const Point& s = (*special)[i];
            int col=(int)((s[0]-xmin)/xr*(width -1));
            int row=(int)((s[1]-ymin)/yr*(height-1));
            row=(height-1)-row;
            col=std::clamp(col,0,width-1);
            row=std::clamp(row,0,height-1);
            std::string cell;
            cell += BOLD;
            cell += COLORS[i%NS];
            cell += '+';
            cell += RESET;
            grid[row][col] = cell;
        }
    }

    std::cout << "\n  " << DIM << "╔";
    for (int i=0;i<width+2;++i) std::cout<<"═";
    std::cout << "╗" << RESET << "\n";
    for (int r=0;r<height;++r) {
        std::cout << "  " << DIM << "║" << RESET << " ";
        for (int c=0;c<width;++c) std::cout<<grid[r][c];
        std::cout << " " << DIM << "║" << RESET << "\n";
    }
    std::cout << "  " << DIM << "╚";
    for (int i=0;i<width+2;++i) std::cout<<"═";
    std::cout << "╝" << RESET << "\n";
    std::cout << "  " << DIM
              << "  " << std::fixed << std::setprecision(1) << xmin
              << std::string(width-6,' ') << xmax << RESET << "\n";
}


// ── sse convergence sparkline ────────────────────────────────────
// renders a mini bar chart of sse over iterations inline

void print_sse_sparkline(const std::vector<double>& history) {
    if (history.empty()) return;
    const std::string bars[] = {"▁","▂","▃","▄","▅","▆","▇","█"};
    double hi = *std::max_element(history.begin(), history.end());
    double lo = *std::min_element(history.begin(), history.end());
    double range = (hi - lo) > 0 ? hi - lo : 1.0;

    std::cout << "  " << DIM << "sse over iterations : " << RESET;
    for (double v : history) {
        int bar = (int)(7.0 * (v - lo) / range);
        bar = std::clamp(bar, 0, 7);
        // color: high sse = red, low sse = green
        std::cout << (bar > 4 ? RED : bar > 1 ? YELLOW : GREEN)
                  << bars[bar] << RESET;
    }
    std::cout << "  " << DIM
              << std::fixed << std::setprecision(2)
              << hi << " → " << lo << RESET << "\n";
}


// ── cluster summary table ────────────────────────────────────────

void print_cluster_table(const std::vector<Point>& pts,
                         const std::vector<int>&   labels,
                         const std::vector<Point>& representatives,
                         int k, const std::string& rep_name) {
    std::cout << "\n  " << std::string(62,'-') << "\n";
    std::cout << "  " << std::left
              << std::setw(10) << "cluster"
              << std::setw(8)  << "symbol"
              << std::setw(8)  << "size"
              << std::setw(24) << rep_name
              << "sse\n";
    std::cout << "  " << std::string(62,'-') << "\n";

    for (int j=0;j<k;++j) {
        std::vector<Point> members;
        for (int i=0;i<(int)pts.size();++i)
            if (labels[i]==j) members.push_back(pts[i]);
        if (members.empty()) continue;

        double sse = 0.0;
        for (const auto& p : members)
            sse += dist_sq(p, representatives[j]);

        std::cout << "  C" << std::left << std::setw(9) << (j+1)
                  << COLORS[j%NS] << SYMBOLS[j%NS] << RESET
                  << std::setw(7) << ""
                  << std::setw(8) << members.size()
                  << "(" << std::fixed << std::setprecision(2)
                  << representatives[j][0] << ", "
                  << representatives[j][1] << ")"
                  << std::string(10,' ')
                  << std::fixed << std::setprecision(4) << sse << "\n";
    }
    std::cout << "  " << std::string(62,'-') << "\n";
}


// ── entry point ──────────────────────────────────────────────────

int main() {
    // dataset — comfortably large for iterative methods (n=24, k=3)
    std::vector<Point> pts = {
        // cluster a — bottom-left
        {1.0,1.2},{1.5,0.8},{1.2,1.5},{0.8,1.0},
        {1.3,0.9},{0.9,1.4},{1.1,1.1},{1.4,1.3},
        // cluster b — top-right
        {5.0,5.3},{5.5,4.8},{4.8,5.1},{5.2,5.6},
        {4.9,4.9},{5.3,5.4},{5.1,5.0},{5.4,5.2},
        // cluster c — top-left
        {1.0,5.0},{1.3,4.7},{0.7,5.2},{1.1,5.5},
        {0.8,4.9},{1.2,5.3},{0.9,5.1},{1.0,4.8},
    };
    int k        = 3;
    int max_iter = 300;
    int n        = (int)pts.size();

    // ── master header ─────────────────────────────────────────────
    std::cout << "\n"
              << "  " << BOLD << "╔══════════════════════════════════════════════════════╗" << RESET << "\n"
              << "  " << BOLD << "║   ITERATIVE IMPROVEMENT K-CLUSTERING  [ n=" << n << ", k=" << k << " ]   ║" << RESET << "\n"
              << "  " << BOLD << "╚══════════════════════════════════════════════════════╝" << RESET << "\n"
              << "\n"
              << "  " << DIM << "covers: lloyd's k-means · k-means++ · k-medoids (pam)" << RESET << "\n"
              << "\n"
              << "  points     → " << n << "\n"
              << "  clusters   → " << k << "\n"
              << "  max iter   → " << max_iter << "\n";


    // ═════════════════════════════════════════════════════════════
    // section a: k-means++ seeding  (section 4.3)
    // ═════════════════════════════════════════════════════════════
    std::cout << "\n  " << BOLD
              << "┌─────────────────────────────────────────────────────┐\n"
              << "  │  A · K-MEANS++ INITIALISATION                       │\n"
              << "  └─────────────────────────────────────────────────────┘"
              << RESET << "\n"
              << "  " << DIM << "  guarantee: o(log k) approximation ratio in expectation" << RESET << "\n\n";

    std::vector<Point> seeds = kmeans_pp_init(pts, k);

    for (int j=0;j<k;++j) {
        std::cout << "  seed " << (j+1) << " : "
                  << COLORS[j%NS] << SYMBOLS[j%NS] << RESET
                  << "  (" << std::fixed << std::setprecision(2)
                  << seeds[j][0] << ", " << seeds[j][1] << ")\n";
    }
    std::cout << "  " << DIM
              << "  seeding ensures spread — prob ∝ D(p)² from nearest chosen centroid"
              << RESET << "\n";


    // ═════════════════════════════════════════════════════════════
    // section b: lloyd's k-means  (section 4.2)
    // ═════════════════════════════════════════════════════════════
    std::cout << "\n  " << BOLD
              << "┌─────────────────────────────────────────────────────┐\n"
              << "  │  B · LLOYD'S K-MEANS                                │\n"
              << "  └─────────────────────────────────────────────────────┘"
              << RESET << "\n"
              << "  " << DIM << "  time: o(n·k·i·d)  |  converges to local optimum" << RESET << "\n\n";

    // run both random-init and k-means++ for comparison
    KMeansResult km_random = kmeans(pts, k, max_iter, false);
    KMeansResult km_pp     = kmeans(pts, k, max_iter, true);

    std::cout << "  " << DIM << "  initialisation comparison:" << RESET << "\n";
    std::cout << "  " << std::string(50,'-') << "\n";
    std::cout << "  " << std::left
              << std::setw(22) << "init method"
              << std::setw(12) << "iterations"
              << "final sse\n";
    std::cout << "  " << std::string(50,'-') << "\n";
    std::cout << "  " << std::setw(22) << "random"
              << std::setw(12) << km_random.iterations
              << std::fixed << std::setprecision(6) << km_random.sse << "\n";
    std::cout << "  " << std::setw(22) << "k-means++"
              << std::setw(12) << km_pp.iterations
              << BOLD << std::fixed << std::setprecision(6) << km_pp.sse
              << RESET << "  ← lower sse\n";
    std::cout << "  " << std::string(50,'-') << "\n";

    // convergence trace (every iteration)
    std::cout << "\n  " << DIM << "  k-means++ convergence trace:" << RESET << "\n";
    for (int i=0;i<(int)km_pp.sse_history.size();++i)
        std::cout << "  " << DIM << "  iter " << std::setw(3) << (i+1)
                  << "  sse = " << std::fixed << std::setprecision(6)
                  << km_pp.sse_history[i] << RESET << "\n";

    std::cout << "\n";
    print_sse_sparkline(km_pp.sse_history);

    // legend
    std::cout << "\n  legend  ( + = centroid ):\n";
    for (int j=0;j<k;++j)
        std::cout << "    " << COLORS[j%NS] << SYMBOLS[j%NS]
                  << RESET << "  →  cluster " << (j+1) << "\n";

    // scatter with centroid markers
    ascii_scatter(pts, km_pp.labels, &km_pp.centroids);
    print_cluster_table(pts, km_pp.labels, km_pp.centroids, k, "centroid");

    // convergence theorem note
    std::cout << "\n  " << DIM
              << "  convergence theorem: sse strictly decreases each iter\n"
              << "  (finite k^n assignments → must terminate)\n"
              << "  worst case: exponential iters (vattani 2011)\n"
              << "  practice: ~o(k) iters on real data" << RESET << "\n";


    // ═════════════════════════════════════════════════════════════
    // section c: k-medoids / pam  (section 4.4)
    // ═════════════════════════════════════════════════════════════
    std::cout << "\n  " << BOLD
              << "┌─────────────────────────────────────────────────────┐\n"
              << "  │  C · K-MEDOIDS  (PAM)                               │\n"
              << "  └─────────────────────────────────────────────────────┘"
              << RESET << "\n"
              << "  " << DIM << "  time: o(n²·k)  |  medoids are real data points\n"
              << "  robust to outliers — works with any distance metric" << RESET << "\n\n";

    PAMResult pam = pam_kmedoids(pts, k);

    std::cout << "  total sse  : " << BOLD
              << std::fixed << std::setprecision(6) << pam.sse << RESET << "\n"
              << "  swaps made : " << pam.swaps << "\n";

    // show medoids
    std::cout << "\n  " << DIM << "  final medoids (actual data points):" << RESET << "\n";
    for (int j=0;j<k;++j)
        std::cout << "  medoid " << (j+1) << " : "
                  << COLORS[j%NS] << SYMBOLS[j%NS] << RESET
                  << "  (" << std::fixed << std::setprecision(2)
                  << pam.medoids[j][0] << ", " << pam.medoids[j][1] << ")"
                  << DIM << "  ← a real point in the dataset" << RESET << "\n";

    // legend
    std::cout << "\n  legend  ( + = medoid ):\n";
    for (int j=0;j<k;++j)
        std::cout << "    " << COLORS[j%NS] << SYMBOLS[j%NS]
                  << RESET << "  →  cluster " << (j+1) << "\n";

    ascii_scatter(pts, pam.labels, &pam.medoids);
    print_cluster_table(pts, pam.labels, pam.medoids, k, "medoid");


    // ═════════════════════════════════════════════════════════════
    // final comparison summary
    // ═════════════════════════════════════════════════════════════
    std::cout << "\n  " << BOLD
              << "┌─────────────────────────────────────────────────────┐\n"
              << "  │  COMPLEXITY COMPARISON  (section 4.5)               │\n"
              << "  └─────────────────────────────────────────────────────┘"
              << RESET << "\n\n";

    std::cout << "  " << std::string(66,'-') << "\n";
    std::cout << "  " << std::left
              << std::setw(22) << "algorithm"
              << std::setw(22) << "time"
              << std::setw(12) << "init"
              << "final sse\n";
    std::cout << "  " << std::string(66,'-') << "\n";
    std::cout << "  " << std::setw(22) << "k-means (random)"
              << std::setw(22) << "o(n·k·i·d)"
              << std::setw(12) << "random"
              << std::fixed << std::setprecision(6) << km_random.sse << "\n";
    std::cout << "  " << std::setw(22) << "k-means++"
              << std::setw(22) << "o(n·k·i·d)"
              << std::setw(12) << "o(n·k)"
              << BOLD << std::fixed << std::setprecision(6) << km_pp.sse
              << RESET << "\n";
    std::cout << "  " << std::setw(22) << "k-medoids (pam)"
              << std::setw(22) << "o(n²·k)"
              << std::setw(12) << "random"
              << std::fixed << std::setprecision(6) << pam.sse << "\n";
    std::cout << "  " << std::string(66,'-') << "\n\n";

    return 0;
}
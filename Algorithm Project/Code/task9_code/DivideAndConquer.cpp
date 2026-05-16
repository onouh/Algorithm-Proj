#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
#include <cmath>
#include <limits>
#include <algorithm>
#include <string>
#include <numeric>
#include <memory>

// ─────────────────────────────────────────────────────────────────
// divide and conquer k-clustering
// time: o(n log n) | space: o(n log n)
// splits point set recursively along widest dimension,
// clusters each half, then merges — may miss cross-cut clusters
// ─────────────────────────────────────────────────────────────────

static const std::string RESET    = "\033[0m";
static const std::string BOLD     = "\033[1m";
static const std::string DIM      = "\033[2m";
static const std::string CYAN     = "\033[96m";
static const std::string YELLOW   = "\033[93m";
static const std::string COLORS[] = { "\033[91m","\033[92m","\033[93m",
                                       "\033[94m","\033[95m","\033[96m" };
static const char        SYMBOLS[] = { '@','#','%','&','*','~' };
static const int         NS = 6;

using Point     = std::array<double, 2>;
using Cluster   = std::vector<Point>;
using Partition = std::vector<Cluster>;


// ── geometry helpers ─────────────────────────────────────────────

Point centroid(const Cluster& c) {
    Point mu = {0.0, 0.0};
    for (const auto& p : c) { mu[0] += p[0]; mu[1] += p[1]; }
    mu[0] /= c.size(); mu[1] /= c.size();
    return mu;
}

double dist_sq(const Point& a, const Point& b) {
    double dx = a[0]-b[0], dy = a[1]-b[1];
    return dx*dx + dy*dy;
}

double cluster_sse(const Cluster& c) {
    Point mu = centroid(c);
    double sse = 0.0;
    for (const auto& p : c)
        sse += dist_sq(p, mu);
    return sse;
}

double total_sse(const Partition& part) {
    double t = 0.0;
    for (const auto& c : part) t += cluster_sse(c);
    return t;
}


// ── quickselect median ───────────────────────────────────────────
// finds the median value along 'axis' in o(n) average time

double quickselect_median(std::vector<Point> pts, int axis) {
    // nth_element rearranges so that pts[mid] is what it would be if sorted
    int mid = (int)pts.size() / 2;
    std::nth_element(pts.begin(), pts.begin() + mid, pts.end(),
        [axis](const Point& a, const Point& b){ return a[axis] < b[axis]; });
    return pts[mid][axis];
}


// ── local refinement near the cut boundary ───────────────────────
// reassigns boundary points to their nearest centroid
// ε controls how wide the boundary band is

void local_refine(Partition& part, const std::vector<Point>& boundary) {
    if (part.empty() || boundary.empty()) return;

    // compute current centroids
    std::vector<Point> centroids;
    centroids.reserve(part.size());
    for (const auto& c : part) {
        if (!c.empty()) centroids.push_back(centroid(c));
    }

    for (const Point& p : boundary) {
        // find which cluster currently holds this point
        int cur_cluster = -1;
        for (int ci = 0; ci < (int)part.size(); ++ci) {
            auto it = std::find(part[ci].begin(), part[ci].end(), p);
            if (it != part[ci].end()) { cur_cluster = ci; break; }
        }
        if (cur_cluster < 0) continue;

        // find nearest centroid
        int    best   = cur_cluster;
        double best_d = dist_sq(p, centroids[cur_cluster]);
        for (int ci = 0; ci < (int)centroids.size(); ++ci) {
            double d = dist_sq(p, centroids[ci]);
            if (d < best_d) { best_d = d; best = ci; }
        }

        // reassign if a closer centroid exists
        if (best != cur_cluster) {
            auto& src = part[cur_cluster];
            src.erase(std::find(src.begin(), src.end(), p));
            part[best].push_back(p);
            // update centroids after move
            if (!part[cur_cluster].empty())
                centroids[cur_cluster] = centroid(part[cur_cluster]);
            centroids[best] = centroid(part[best]);
        }
    }
    // remove any empty clusters that may have formed
    part.erase(std::remove_if(part.begin(), part.end(),
        [](const Cluster& c){ return c.empty(); }), part.end());
}


// ── kd-tree ──────────────────────────────────────────────────────
// build: o(n log n) | nn-query: o(log n) avg, o(n) worst
// cycles through dimensions at each depth level

struct KDNode {
    Point                    point;
    int                      axis;
    std::unique_ptr<KDNode>  left, right;
};

std::unique_ptr<KDNode> build_kdtree(std::vector<Point> pts, int depth = 0) {
    // algorithm BuildKDTree(P, depth) from section 3.4
    if (pts.empty()) return nullptr;

    int axis = depth % 2;   // cycle through x=0, y=1
    std::sort(pts.begin(), pts.end(),
        [axis](const Point& a, const Point& b){ return a[axis] < b[axis]; });

    int mid = (int)pts.size() / 2;
    auto node        = std::make_unique<KDNode>();
    node->point      = pts[mid];
    node->axis       = axis;
    node->left       = build_kdtree(std::vector<Point>(pts.begin(), pts.begin()+mid), depth+1);
    node->right      = build_kdtree(std::vector<Point>(pts.begin()+mid+1, pts.end()), depth+1);
    return node;
}

void kdtree_nearest(const KDNode* node, const Point& query,
                    Point& best, double& best_dist) {
    if (!node) return;
    double d = dist_sq(query, node->point);
    if (d < best_dist) { best_dist = d; best = node->point; }

    int axis  = node->axis;
    double diff = query[axis] - node->point[axis];
    // go into the near side first, then check far side if needed
    const KDNode* near = diff <= 0 ? node->left.get()  : node->right.get();
    const KDNode* far  = diff <= 0 ? node->right.get() : node->left.get();
    kdtree_nearest(near, query, best, best_dist);
    if (diff * diff < best_dist)
        kdtree_nearest(far,  query, best, best_dist);
}


// ── main divide and conquer algorithm ────────────────────────────
// algorithm DivideConquer_KClustering(P, k) — section 3.2

static int  recursion_depth = 0;   // track depth for logging
static bool verbose         = true;

Partition dc_clustering(std::vector<Point> pts, int k, double eps = 0.4) {
    int n = (int)pts.size();

    // base case: <= k points — each is its own cluster (line 1-3)
    if (n <= k) {
        Partition result;
        for (const auto& p : pts) result.push_back({p});
        return result;
    }

    // base case: k=1 — single cluster (line 4-6)
    if (k == 1) return { pts };

    // ── divide: find widest-spread dimension (line 7-8) ──────────
    double spread_x = 0.0, spread_y = 0.0;
    {
        double xmin=1e9,xmax=-1e9,ymin=1e9,ymax=-1e9;
        for (const auto& p : pts) {
            xmin=std::min(xmin,p[0]); xmax=std::max(xmax,p[0]);
            ymin=std::min(ymin,p[1]); ymax=std::max(ymax,p[1]);
        }
        spread_x = xmax - xmin;
        spread_y = ymax - ymin;
    }
    int axis = (spread_x >= spread_y) ? 0 : 1;

    // line 9: quickselect median along chosen axis
    double median = quickselect_median(pts, axis);

    if (verbose) {
        std::string indent(recursion_depth * 2, ' ');
        std::string ax = axis == 0 ? "x" : "y";
        std::cout << "  " << DIM << indent
                  << "depth=" << recursion_depth
                  << "  n=" << n << "  k=" << k
                  << "  split on " << ax << "-axis  median=" 
                  << std::fixed << std::setprecision(2) << median
                  << RESET << "\n";
    }

    // lines 10-11: partition into left and right halves
    std::vector<Point> p_left, p_right;
    for (const auto& p : pts) {
        if (p[axis] <= median) p_left.push_back(p);
        else                   p_right.push_back(p);
    }
    // guard: if all points land on one side, force a split
    if (p_left.empty())  { p_left.push_back(p_right.back()); p_right.pop_back(); }
    if (p_right.empty()) { p_right.push_back(p_left.back());  p_left.pop_back(); }

    // lines 12-14: allocate k proportionally to each half
    int k_left  = std::max(1, (int)std::round((double)k * p_left.size() / n));
    int k_right = k - k_left;
    if (k_right < 1) { k_right = 1; k_left = k - 1; }

    // lines 15-17: conquer — recurse on each half
    ++recursion_depth;
    Partition c_left  = dc_clustering(p_left,  k_left,  eps);
    Partition c_right = dc_clustering(p_right, k_right, eps);
    --recursion_depth;

    // lines 18-19: merge both sub-partitions
    Partition merged;
    merged.insert(merged.end(), c_left.begin(),  c_left.end());
    merged.insert(merged.end(), c_right.begin(), c_right.end());

    // lines 20-22: local refinement on boundary points near the cut
    std::vector<Point> boundary;
    for (const auto& p : pts)
        if (std::abs(p[axis] - median) < eps)
            boundary.push_back(p);
    if (!boundary.empty())
        local_refine(merged, boundary);

    return merged;
}


// ── build and print kd-tree structure ───────────────────────────

void print_kdtree(const KDNode* node, const std::string& prefix = "",
                  bool is_left = true, int depth = 0) {
    if (!node) return;
    std::string ax = node->axis == 0 ? "x" : "y";
    std::cout << "  " << DIM << prefix
              << (is_left ? "├─ " : "└─ ") << RESET
              << CYAN << "(" << std::fixed << std::setprecision(1)
              << node->point[0] << "," << node->point[1] << ")" << RESET
              << DIM << " [split " << ax << "]" << RESET << "\n";
    std::string child_prefix = prefix + (is_left ? "│  " : "   ");
    print_kdtree(node->left.get(),  child_prefix, true,  depth+1);
    print_kdtree(node->right.get(), child_prefix, false, depth+1);
}


// ── assign cluster labels to original points ─────────────────────
// each point → index of the cluster whose centroid is nearest

std::vector<int> assign_labels(const std::vector<Point>& points,
                                const Partition& partition) {
    int k = (int)partition.size();
    std::vector<Point> centroids(k);
    for (int i = 0; i < k; ++i) centroids[i] = centroid(partition[i]);

    std::vector<int> labels(points.size());
    for (int i = 0; i < (int)points.size(); ++i) {
        int    best   = 0;
        double best_d = dist_sq(points[i], centroids[0]);
        for (int j = 1; j < k; ++j) {
            double d = dist_sq(points[i], centroids[j]);
            if (d < best_d) { best_d = d; best = j; }
        }
        labels[i] = best;
    }
    return labels;
}


// ── ascii scatter plot ───────────────────────────────────────────

void ascii_scatter(const std::vector<Point>& points,
                   const std::vector<int>& labels,
                   int k, int width = 62, int height = 20) {
    double xmin=1e9,xmax=-1e9,ymin=1e9,ymax=-1e9;
    for (const auto& p : points) {
        xmin=std::min(xmin,p[0]); xmax=std::max(xmax,p[0]);
        ymin=std::min(ymin,p[1]); ymax=std::max(ymax,p[1]);
    }
    double xr = xmax-xmin > 0 ? xmax-xmin : 1.0;
    double yr = ymax-ymin > 0 ? ymax-ymin : 1.0;

    std::vector<std::vector<std::string>> grid(height,
        std::vector<std::string>(width, " "));

    for (int i = 0; i < (int)points.size(); ++i) {
        int col = (int)((points[i][0]-xmin)/xr*(width -1));
        int row = (int)((points[i][1]-ymin)/yr*(height-1));
        row     = (height-1) - row;
        col     = std::clamp(col, 0, width -1);
        row     = std::clamp(row, 0, height-1);
        int lbl = labels[i];
        std::string cell;
        cell += COLORS[lbl % NS];
        cell += SYMBOLS[lbl % NS];
        cell += RESET;
        grid[row][col] = cell;
    }

    std::cout << "\n  " << DIM << "╔";
    for (int i=0;i<width+2;++i) std::cout<<"═";
    std::cout << "╗" << RESET << "\n";

    for (int r=0;r<height;++r) {
        std::cout << "  " << DIM << "║" << RESET << " ";
        for (int c=0;c<width;++c) std::cout << grid[r][c];
        std::cout << " " << DIM << "║" << RESET << "\n";
    }

    std::cout << "  " << DIM << "╚";
    for (int i=0;i<width+2;++i) std::cout<<"═";
    std::cout << "╝" << RESET << "\n";

    std::cout << "  " << DIM
              << "  " << std::fixed << std::setprecision(1) << xmin
              << std::string(width-6,' ') << xmax << RESET << "\n";
}


// ── cluster summary table ────────────────────────────────────────

void print_cluster_table(const Partition& part, int k) {
    std::cout << "\n  " << std::string(58,'-') << "\n";
    std::cout << "  " << std::left
              << std::setw(10) << "cluster"
              << std::setw(8)  << "symbol"
              << std::setw(8)  << "size"
              << std::setw(22) << "centroid"
              << "sse\n";
    std::cout << "  " << std::string(58,'-') << "\n";
    for (int i=0;i<(int)part.size();++i) {
        if (part[i].empty()) continue;
        Point  mu  = centroid(part[i]);
        double sse = cluster_sse(part[i]);
        std::cout << "  C" << std::left << std::setw(9) << (i+1)
                  << COLORS[i%NS] << SYMBOLS[i%NS] << RESET
                  << std::setw(7) << ""
                  << std::setw(8) << part[i].size()
                  << "(" << std::fixed << std::setprecision(2)
                  << mu[0] << ", " << mu[1] << ")"
                  << std::string(10,' ')
                  << std::fixed << std::setprecision(4) << sse << "\n";
    }
    std::cout << "  " << std::string(58,'-') << "\n";
}


// ── entry point ──────────────────────────────────────────────────

int main() {
    // dataset — larger than brute force can handle (n=16, k=3)
    std::vector<Point> points = {
        {1.0,1.2},{1.5,0.8},{1.2,1.5},{0.8,1.0},{1.3,0.9},{0.9,1.4},
        {5.0,5.3},{5.5,4.8},{4.8,5.1},{5.2,5.6},{4.9,4.9},{5.3,5.4},
        {1.0,5.0},{1.3,4.7},{0.7,5.2},{1.1,5.5},
    };
    int    k   = 3;
    double eps = 0.5;   // boundary band width for local refinement
    int    n   = (int)points.size();

    // ── header ────────────────────────────────────────────────────
    std::cout << "\n"
              << "  " << BOLD << "╔══════════════════════════════════════════════════════╗" << RESET << "\n"
              << "  " << BOLD << "║     DIVIDE & CONQUER K-CLUSTERING  [ n=" << n << ", k=" << k << " ]      ║" << RESET << "\n"
              << "  " << BOLD << "╚══════════════════════════════════════════════════════╝" << RESET << "\n"
              << "\n"
              << "  " << DIM  << "algorithm : recursive median split + local refinement" << RESET << "\n"
              << "  " << DIM  << "complexity : o(n log n)  — master theorem case 2"     << RESET << "\n"
              << "  " << DIM  << "guarantee  : approximate  (may miss cross-cut clusters)" << RESET << "\n"
              << "\n"
              << "  points         → " << n << "\n"
              << "  clusters       → " << k << "\n"
              << "  boundary eps   → " << eps << "\n";

    // ── section a: kd-tree ────────────────────────────────────────
    std::cout << "\n  " << BOLD << "▸ BUILDING KD-TREE" << RESET
              << DIM << "  (build: o(n log n)  |  nn-query: o(log n) avg)" << RESET << "\n\n";

    auto kdroot = build_kdtree(points);
    print_kdtree(kdroot.get(), "  ", false);

    // demonstrate nearest-neighbour query
    Point query = {3.0, 3.0};
    Point nn    = points[0];
    double best_d = std::numeric_limits<double>::infinity();
    kdtree_nearest(kdroot.get(), query, nn, best_d);
    std::cout << "\n  " << DIM
              << "  nn query for (3.0,3.0)  →  nearest = ("
              << std::fixed << std::setprecision(1) << nn[0] << "," << nn[1]
              << ")  dist=" << std::sqrt(best_d) << RESET << "\n";

    // ── section b: d&c clustering ─────────────────────────────────
    std::cout << "\n  " << BOLD << "▸ DIVIDE & CONQUER RECURSION" << RESET << "\n\n";

    Partition result = dc_clustering(points, k, eps);

    // pad to exactly k clusters if refinement collapsed any
    while ((int)result.size() < k) result.push_back({});

    std::vector<int> labels = assign_labels(points, result);

    // ── results ───────────────────────────────────────────────────
    std::cout << "\n  " << BOLD << "▸ PARTITION FOUND" << RESET << "\n"
              << "  total sse   : " << BOLD
              << std::fixed << std::setprecision(6) << total_sse(result)
              << RESET << "\n";

    // ── legend ────────────────────────────────────────────────────
    std::cout << "\n  legend :\n";
    for (int i=0;i<(int)result.size();++i)
        std::cout << "    " << COLORS[i%NS] << SYMBOLS[i%NS]
                  << RESET << "  →  cluster " << (i+1) << "\n";

    // ── scatter plot ──────────────────────────────────────────────
    ascii_scatter(points, labels, k);

    // ── cluster table ─────────────────────────────────────────────
    print_cluster_table(result, k);

    // ── bfr sufficient statistics note ───────────────────────────
    std::cout << "\n  " << BOLD << "▸ BFR SUFFICIENT STATISTICS  (section 3.5)" << RESET << "\n"
              << "  " << DIM << "  per-cluster summary stored as (N, SUM, SUMSQ)" << RESET << "\n\n";

    std::cout << "  " << std::string(58,'-') << "\n";
    std::cout << "  " << std::left
              << std::setw(10) << "cluster"
              << std::setw(8)  << "N"
              << std::setw(20) << "SUM (x,y)"
              << "SUMSQ (x,y)\n";
    std::cout << "  " << std::string(58,'-') << "\n";

    for (int i=0;i<(int)result.size();++i) {
        const auto& cl = result[i];
        if (cl.empty()) continue;
        int    N     = (int)cl.size();
        double sx=0,sy=0,sx2=0,sy2=0;
        for (const auto& p : cl) {
            sx  += p[0]; sy  += p[1];
            sx2 += p[0]*p[0]; sy2 += p[1]*p[1];
        }
        // centroid: mu = SUM/N | variance: s2 = SUMSQ/N - (SUM/N)^2
        std::cout << "  C" << std::left << std::setw(9) << (i+1)
                  << std::setw(8) << N
                  << "(" << std::fixed << std::setprecision(2)
                  << sx << ", " << sy << ")    "
                  << "(" << sx2 << ", " << sy2 << ")\n";
    }
    std::cout << "  " << std::string(58,'-') << "\n";
    std::cout << "  " << DIM
              << "  memory: o(k·d)  vs  o(n·d) for raw storage"
              << RESET << "\n";

    // ── complexity note ───────────────────────────────────────────
    std::cout << "\n  " << DIM << "⚠  may miss clusters that span the partition cut" << RESET << "\n"
              << "  " << DIM << "   local refinement (ε=" << eps << ") partially corrects this" << RESET << "\n\n";

    return 0;
}
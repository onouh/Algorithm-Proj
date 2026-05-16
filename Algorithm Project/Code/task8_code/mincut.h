#pragma once
#include <vector>
#include <string>
#include <climits>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <random>
#include <functional>

// ── Graph representation ──────────────────────────────────────────────────────
struct Edge {
    int   u, v;
    double w;
};

struct Graph {
    int n = 0;
    std::vector<std::vector<double>> adj;  // adjacency matrix

    Graph() = default;
    explicit Graph(int n_) : n(n_), adj(n_, std::vector<double>(n_, 0.0)) {}

    void addEdge(int u, int v, double w) {
        adj[u][v] = w;
        adj[v][u] = w;
    }

    std::vector<Edge> edges() const {
        std::vector<Edge> result;
        for (int i = 0; i < n; i++)
            for (int j = i+1; j < n; j++)
                if (adj[i][j] > 0)
                    result.push_back({i, j, adj[i][j]});
        return result;
    }
};

// ── Cut result ────────────────────────────────────────────────────────────────
struct CutResult {
    double             cutWeight  = 0.0;
    std::vector<int>   setA;
    std::vector<int>   setB;
    std::vector<std::string> log;
    std::string        technique;
};

// ── Shared helper: compute cut weight for a given partition ───────────────────
inline double computeCut(const Graph& G, const std::vector<bool>& inA)
{
    double cut = 0.0;
    for (int i = 0; i < G.n; i++)
        for (int j = i+1; j < G.n; j++)
            if (inA[i] != inA[j])
                cut += G.adj[i][j];
    return cut;
}

// ── Shared helper: format a set as "{0,1,2}" ─────────────────────────────────
inline std::string fmtSet(const std::vector<int>& s) {
    std::string r = "{";
    for (int i = 0; i < (int)s.size(); i++) {
        if (i) r += ",";
        r += std::to_string(s[i]);
    }
    r += "}";
    return r;
}

// ─────────────────────────────────────────────────────────────────────────────
// TECHNIQUE A: BRUTE FORCE
// Exhaustively tests every partition of V into non-empty (A, B).
// Vertex 0 is fixed in A to avoid counting mirror partitions twice.
// Complexity: O(2^(n-1) * n^2)
// ─────────────────────────────────────────────────────────────────────────────
inline CutResult bruteForce(const Graph& G) {
    CutResult res;
    res.technique = "Brute Force";
    res.cutWeight = 1e18;

    res.log.push_back("=== BRUTE FORCE MIN CUT ===");
    res.log.push_back("Enumerating all 2^(n-1) - 1 partitions...");
    res.log.push_back("(Vertex 0 is fixed in set A to avoid duplicates)");
    res.log.push_back("----------------------------");

    // We enumerate all non-zero masks over vertices 1..n-1.
    // Bit (i-1) set  => vertex i is in A.
    // Bit (i-1) clear => vertex i is in B.
    // We skip masks where B would be empty (all bits set => all in A).
    int fullMask = (1 << (G.n - 1)) - 1;  // all vertices 1..n-1 in A, B empty
    int checked  = 0;

    for (int mask = 1; mask <= fullMask; mask++) {
        // Skip the case where B is empty (mask == fullMask means every vertex
        // other than 0 is in A, leaving B completely empty).
        if (mask == fullMask) continue;

        std::vector<bool> inA(G.n, false);
        inA[0] = true;  // vertex 0 always in A
        for (int i = 1; i < G.n; i++) {
            if (mask & (1 << (i - 1))) inA[i] = true;
        }

        double cut = computeCut(G, inA);
        checked++;

        // Build log line
        std::vector<int> sA, sB;
        for (int i = 0; i < G.n; i++) (inA[i] ? sA : sB).push_back(i);
        std::string line = "Partition " + fmtSet(sA) + " | " + fmtSet(sB)
                         + "  ->  cut = " + std::to_string((int)cut);

        if (cut < res.cutWeight) {
            res.cutWeight = cut;
            res.setA = sA;
            res.setB = sB;
            line += "  <-- new best";
        }
        res.log.push_back(line);
    }

    res.log.push_back("----------------------------");
    res.log.push_back("Total partitions checked: " + std::to_string(checked));
    res.log.push_back("MINIMUM CUT = " + std::to_string((int)res.cutWeight));
    res.log.push_back("Set A = " + fmtSet(res.setA));
    res.log.push_back("Set B = " + fmtSet(res.setB));
    return res;
}

// ─────────────────────────────────────────────────────────────────────────────
// TECHNIQUE B: KERNIGHAN-LIN (Iterative Improvement)
//
// Each pass:
//   1. Compute D[v] = ExternalCost(v) - InternalCost(v) for every vertex.
//   2. Greedily pick n/2 non-overlapping swap pairs (a∈A, b∈B) maximising
//      gain(a,b) = D[a] + D[b] - 2*w[a][b].  Lock chosen vertices and update
//      remaining D values after each selection.
//   3. Apply the prefix of swaps with the best cumulative gain.
//   4. Repeat until no positive-gain prefix exists (local optimum).
//
// Random restarts escape different local optima.
// Complexity per pass: O(n^2).  Total: O(restarts * passes * n^2).
// ─────────────────────────────────────────────────────────────────────────────
inline CutResult kernighanLin(const Graph& G, int restarts = 5) {
    CutResult best;
    best.technique = "Kernighan-Lin (Iterative Improvement)";
    best.cutWeight = 1e18;

    best.log.push_back("=== KERNIGHAN-LIN ALGORITHM ===");
    best.log.push_back("Restarts: " + std::to_string(restarts));
    best.log.push_back("-------------------------------");

    std::mt19937 rng(42);

    for (int restart = 0; restart < restarts; restart++) {
        // ── Random balanced initial partition ─────────────────────────────
        std::vector<int> perm(G.n);
        std::iota(perm.begin(), perm.end(), 0);
        std::shuffle(perm.begin(), perm.end(), rng);

        std::vector<bool> inA(G.n, false);
        int half = G.n / 2;
        for (int i = 0; i < half; i++) inA[perm[i]] = true;

        best.log.push_back("--- Restart " + std::to_string(restart + 1) + " ---");

        // ── Iterative improvement passes ──────────────────────────────────
        for (int pass = 1; pass <= 200; pass++) {

            // Step 1: compute D values from scratch
            std::vector<double> D(G.n, 0.0);
            for (int v = 0; v < G.n; v++) {
                for (int u = 0; u < G.n; u++) {
                    if (u == v) continue;
                    if (inA[v] != inA[u]) D[v] += G.adj[v][u];   // external
                    else                  D[v] -= G.adj[v][u];    // internal
                }
            }

            // Step 2: find up to n/2 sequential best-gain swap pairs
            std::vector<bool>              locked(G.n, false);
            std::vector<double>            gains;
            std::vector<std::pair<int,int>> pairs;

            int maxPairs = G.n / 2;
            for (int step = 0; step < maxPairs; step++) {
                double bestGain = -1e18;
                int    ba = -1, bb = -1;

                for (int a = 0; a < G.n; a++) {
                    if (!inA[a] || locked[a]) continue;
                    for (int b = 0; b < G.n; b++) {
                        if (inA[b]  || locked[b]) continue;
                        double g = D[a] + D[b] - 2.0 * G.adj[a][b];
                        if (g > bestGain) { bestGain = g; ba = a; bb = b; }
                    }
                }
                if (ba < 0 || bb < 0) break;

                gains.push_back(bestGain);
                pairs.push_back({ba, bb});
                locked[ba] = locked[bb] = true;

                // Step 3: update D values for unlocked vertices, reflecting
                // the tentative move of ba (A->B) and bb (B->A).
                for (int v = 0; v < G.n; v++) {
                    if (locked[v]) continue;
                    if (inA[v]) {
                        // v is in A; ba leaves A, bb joins A
                        D[v] += 2.0 * G.adj[v][ba]   // ba no longer external for A-vertices
                              - 2.0 * G.adj[v][bb];   // bb now internal for A-vertices
                    } else {
                        // v is in B; bb leaves B, ba joins B
                        D[v] += 2.0 * G.adj[v][bb]   // bb no longer external for B-vertices
                              - 2.0 * G.adj[v][ba];   // ba now internal for B-vertices
                    }
                }
            }

            // Step 4: find prefix 1..k with maximum cumulative gain
            double cumul = 0.0, maxCumul = -1e18;
            int    bestK = 0;
            for (int i = 0; i < (int)gains.size(); i++) {
                cumul += gains[i];
                if (cumul > maxCumul) { maxCumul = cumul; bestK = i + 1; }
            }

            std::string passLine = "Pass " + std::to_string(pass)
                + ": best_k=" + std::to_string(bestK)
                + "  gain=" + std::to_string((int)maxCumul);

            if (maxCumul > 1e-9 && bestK > 0) {
                // Apply the first bestK swaps:
                // pairs[i].first  is in A  -> move to B
                // pairs[i].second is in B  -> move to A
                for (int i = 0; i < bestK; i++) {
                    inA[pairs[i].first]  = false;   // a leaves A
                    inA[pairs[i].second] = true;    // b joins A
                }
                best.log.push_back(passLine + "  -> swapped " + std::to_string(bestK) + " pairs");
            } else {
                best.log.push_back(passLine + "  -> local optimum reached");
                break;
            }
        }

        double cut = computeCut(G, inA);
        best.log.push_back("Restart " + std::to_string(restart + 1)
            + " final cut = " + std::to_string((int)cut));

        if (cut < best.cutWeight) {
            best.cutWeight = cut;
            best.setA.clear(); best.setB.clear();
            for (int i = 0; i < G.n; i++)
                (inA[i] ? best.setA : best.setB).push_back(i);
        }
    }

    best.log.push_back("-------------------------------");
    best.log.push_back("BEST CUT FOUND = " + std::to_string((int)best.cutWeight));
    best.log.push_back("Set A = " + fmtSet(best.setA));
    best.log.push_back("Set B = " + fmtSet(best.setB));
    return best;
}

// ─────────────────────────────────────────────────────────────────────────────
// TECHNIQUE C: STOER-WAGNER (exact, polynomial)
//
// Performs n-1 phases.  Each phase runs a Maximum-Adjacency Ordering (MAO)
// over the currently active super-vertices and records the phase cut value
// as the weight of the last vertex added (the sum of its edge weights to
// already-added vertices).  After each phase the two last vertices are merged.
// The global minimum cut is the minimum over all phase cuts.
//
// Complexity: O(n^3) with adjacency matrix.
// ─────────────────────────────────────────────────────────────────────────────
inline CutResult stoerWagner(const Graph& G) {
    CutResult res;
    res.technique = "Stoer-Wagner (Exact)";
    res.cutWeight = 1e18;

    res.log.push_back("=== STOER-WAGNER ALGORITHM ===");
    res.log.push_back("Exact minimum cut — O(n^3)");
    res.log.push_back("------------------------------");

    int n = G.n;

    // w[i][j]: contracted edge weights (may be multi-edges after merging)
    std::vector<std::vector<double>> w = G.adj;

    // merged[i] = list of original vertex indices represented by super-vertex i
    std::vector<std::vector<int>> merged(n);
    for (int i = 0; i < n; i++) merged[i] = {i};

    std::vector<bool> active(n, true);

    for (int phase = 0; phase < n - 1; phase++) {

        // Collect all currently active super-vertices
        std::vector<int> active_verts;
        for (int i = 0; i < n; i++) if (active[i]) active_verts.push_back(i);

        int numActive = (int)active_verts.size();

        // Maximum-Adjacency Ordering (MAO)
        // key[v] = total weight of edges from v to already-added vertices in A
        std::vector<double> key(n, 0.0);
        std::vector<bool>   inMAO(n, false);

        int prev = -1, last = -1;

        for (int iter = 0; iter < numActive; iter++) {
            // Pick the active vertex not yet in MAO with the highest key
            double maxKey = -1e18;
            int    z = -1;
            for (int v : active_verts) {
                if (!inMAO[v] && key[v] > maxKey) { maxKey = key[v]; z = v; }
            }

            // On the very first iteration all keys are 0; pick any active vertex
            if (z < 0) {
                for (int v : active_verts) {
                    if (!inMAO[v]) { z = v; break; }
                }
            }

            inMAO[z] = true;
            prev = last;
            last = z;

            // Update keys: add edges from z to not-yet-added active vertices
            for (int v : active_verts) {
                if (!inMAO[v]) key[v] += w[z][v];
            }
        }

        // The phase cut value equals key[last] = total weight of edges
        // from 'last' to all other vertices in the MAO ordering (i.e. the
        // cut that separates {last} from everything else in this phase).
        double phaseCut = key[last];

        res.log.push_back("Phase " + std::to_string(phase + 1)
            + ": s=" + std::to_string(prev)
            + " t=" + std::to_string(last)
            + "  phase_cut=" + std::to_string((int)phaseCut));

        if (phaseCut < res.cutWeight) {
            res.cutWeight = phaseCut;
            // The minimum cut for this phase puts all original vertices
            // that are currently merged into 'last' on one side, everything
            // else on the other.
            res.setA.clear(); res.setB.clear();
            // Build a set of original vertices in merged[last]
            std::vector<bool> inLast(G.n, false);
            for (int orig : merged[last]) inLast[orig] = true;
            for (int i = 0; i < G.n; i++)
                (inLast[i] ? res.setA : res.setB).push_back(i);
            res.log.push_back("  -> new best cut = " + std::to_string((int)phaseCut));
        }

        // Merge 'last' into 'prev': combine their edge weights and
        // transfer the original-vertex membership.
        if (prev >= 0) {
            for (int v = 0; v < n; v++) {
                w[prev][v] += w[last][v];
                w[v][prev] += w[v][last];
            }
            w[prev][prev] = 0.0;  // no self-loops
            for (int orig : merged[last]) merged[prev].push_back(orig);
            active[last] = false;
        }
    }

    res.log.push_back("------------------------------");
    res.log.push_back("EXACT MINIMUM CUT = " + std::to_string((int)res.cutWeight));
    res.log.push_back("Set A = " + fmtSet(res.setA));
    res.log.push_back("Set B = " + fmtSet(res.setB));
    return res;
}

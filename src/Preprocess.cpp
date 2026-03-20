// ============================================================
// Preprocess.cpp
// ============================================================

#include "../include/Preprocess.h"
#include "../include/BFSAlgorithms.h"

// Helper: build the initial BFSState from G_0
static BFSState buildInitialState(int numVertices, int source,
                                  const EdgeList& initialEdges)
{
    BFSState state(numVertices, source);
    for (const auto& e : initialEdges) {
        state.addEdge(e.u, e.v);
    }
    state.computeBFS();
    return state;
}

// ===========================================================
// Incremental preprocessing
// Run classical incremental BFS on the predicted insertions.
// ===========================================================
std::vector<BFSSnapshot> preprocessIncremental(
    int             numVertices,
    int             source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates)
{
    BFSState state = buildInitialState(numVertices, source, initialEdges);
    int m = (int)predictedUpdates.size();
    std::vector<BFSSnapshot> snapshots(m + 1);
    snapshots[0] = state.saveSnapshot(/*saveUP=*/false);

    for (int i = 0; i < m; ++i) {
        const auto& e = predictedUpdates[i];
        // All updates must be insertions in the incremental setting
        if (e.type == UpdateType::INSERT) {
            classicalInsertEdge(state, e.u, e.v);
        }
        // If a deletion appears in a supposedly incremental sequence,
        // we skip it (defensive programming).
        snapshots[i + 1] = state.saveSnapshot(/*saveUP=*/false);
    }
    return snapshots;
}

// ===========================================================
// Decremental preprocessing
// Run classical decremental BFS on the predicted deletions.
// ===========================================================
std::vector<BFSSnapshot> preprocessDecremental(
    int             numVertices,
    int             source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates)
{
    BFSState state = buildInitialState(numVertices, source, initialEdges);
    state.computeAllUP(); // UP sets required for decremental repair

    int m = (int)predictedUpdates.size();
    std::vector<BFSSnapshot> snapshots(m + 1);
    snapshots[0] = state.saveSnapshot(/*saveUP=*/true);

    for (int i = 0; i < m; ++i) {
        const auto& e = predictedUpdates[i];
        if (e.type == UpdateType::DELETE) {
            classicalDeleteEdge(state, e.u, e.v);
        }
        snapshots[i + 1] = state.saveSnapshot(/*saveUP=*/true);
    }
    return snapshots;
}

// ===========================================================
// Fully-dynamic preprocessing
// After each predicted update, run a full BFS from scratch.
// O(m^2) total time; simpler and always correct for mixed updates.
// ===========================================================
std::vector<BFSSnapshot> preprocessFullyDynamic(
    int             numVertices,
    int             source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates)
{
    BFSState state = buildInitialState(numVertices, source, initialEdges);
    state.computeAllUP();

    int m = (int)predictedUpdates.size();
    std::vector<BFSSnapshot> snapshots(m + 1);
    snapshots[0] = state.saveSnapshot(/*saveUP=*/true);

    for (int i = 0; i < m; ++i) {
        const auto& e = predictedUpdates[i];
        if (e.type == UpdateType::INSERT) {
            state.addEdge(e.u, e.v);
        } else {
            state.removeEdge(e.u, e.v);
        }
        state.computeBFS();
        state.computeAllUP();
        snapshots[i + 1] = state.saveSnapshot(/*saveUP=*/true);
    }
    return snapshots;
}

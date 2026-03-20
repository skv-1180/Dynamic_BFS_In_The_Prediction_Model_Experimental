#include "../include/Preprocess.h"
#include "../include/BFSAlgorithms.h"

static BFSState buildInitialState(int numVertices, int source,
                                  const EdgeList& initialEdges) {
    BFSState state(numVertices, source);
    for (const auto& e : initialEdges) {
        state.addEdge(e.u, e.v);
    }
    state.computeBFS();
    return state;
}

std::vector<BFSSnapshot> preprocessIncremental(
    int numVertices,
    int source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates) {
    BFSState state = buildInitialState(numVertices, source, initialEdges);
    int m = (int)predictedUpdates.size();
    std::vector<BFSSnapshot> snapshots(m + 1);
    snapshots[0] = state.saveSnapshot(false);

    for (int i = 0; i < m; ++i) {
        const auto& e = predictedUpdates[i];
        if (e.type == UpdateType::INSERT) {
            classicalInsertEdge(state, e.u, e.v);
        }
        snapshots[i + 1] = state.saveSnapshot(/*saveUP=*/false);
    }
    return snapshots;
}

std::vector<BFSSnapshot> preprocessDecremental(
    int numVertices,
    int source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates) {
    BFSState state = buildInitialState(numVertices, source, initialEdges);
    state.computeAllUP();
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

std::vector<BFSSnapshot> preprocessFullyDynamic(
    int numVertices,
    int source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates) {
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

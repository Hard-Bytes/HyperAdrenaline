#pragma once
#include "graphNode.hpp"
#include <vector>

//template <class T>
class HeapGraph
{
public:
    // Constructor
    explicit HeapGraph( int maxNodes);
    ~HeapGraph();

    void addNode(GraphNode* newNode) noexcept;
    GraphNode* recoverFirst() noexcept;
    bool contains(GraphNode* node) const noexcept;

    int getSize() const noexcept;

private:

    void sortUp(int itemIndex) noexcept;
    void sortDown(int itemIndex) noexcept;
    std::vector<GraphNode*> tree;

};

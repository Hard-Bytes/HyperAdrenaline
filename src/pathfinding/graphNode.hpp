#pragma once
#include <list>
#include "triangle.hpp"
#include "../facade/vector3f.hpp"

class GraphNode
{
    public:
        //functions
        GraphNode();
        GraphNode(Triangle triangle, Vector3f centerPoint);
        ~GraphNode();

        //Variables
        Triangle triangle;
        std::list<GraphNode*> neighbours;
        Vector3f center;
        GraphNode* previous{nullptr};
        int fCost{INT_MAX}; // Final cost
        int gCost{INT_MAX}; // Cost from start to this node
        int hCost{0};       // Estimated cost to end
        int index{0};

        // Overload operators
        GraphNode& operator=(const GraphNode&);
        bool operator==(const GraphNode&);

        //Functions
        bool isNeightbour(Triangle *t);
        GraphNode* reset();

    private:
};

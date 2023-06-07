#include "graphNode.hpp"

GraphNode::GraphNode()
:triangle(),center()
{
}

GraphNode::GraphNode(Triangle triangle, Vector3f centerPoint)
: triangle(triangle),center(centerPoint)
{
}

GraphNode::~GraphNode()
{
    neighbours.clear();
    previous = nullptr;
}

/*Operator =*/
// Overload of the assign operator
GraphNode& GraphNode::operator=(const GraphNode &g)
{
    if(this->index==g.index)return *this;
    this->neighbours = g.neighbours;
    this->center = g.center;
    this->fCost = g.fCost;
    this->gCost = g.gCost;
    this->hCost = g.hCost;
    this->triangle = g.triangle;
    this->previous = g.previous;
    return *this;
}

/*Operator ==*/
// Overload of the comparison operator
bool GraphNode::operator==(const GraphNode &g)
{
    if(this->index == g.index)  return true;
    return false;

}

/*GraphNode: isNeightbour*/
//Returns true if the given triangle shares two vertex with this
bool GraphNode::isNeightbour(Triangle *t)
{
    return this->triangle.isNeighbour(t);
}

/*GraphNode: reset*/
// Resets the values of F,G and H cost and previous to their initial values
// Return previous
// Used when retracing the path of the AI
GraphNode* GraphNode::reset()
{
    this->fCost = INT_MAX;
    this->gCost = INT_MAX;
    this->hCost = 0;

    GraphNode* aux = this->previous;
    this->previous = nullptr;
    return aux;
} 


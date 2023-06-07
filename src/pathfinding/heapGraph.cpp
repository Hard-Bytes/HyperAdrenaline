#include "heapGraph.hpp"
#include <algorithm>


/*Contructor*/
// We create a Heap of max size equal to maxNodes
HeapGraph:: HeapGraph(int maxNodes)
{
    tree.reserve(maxNodes);
}

/*Destructor*/
HeapGraph::~HeapGraph()
{
    tree.clear();
}

/*Add Node*/
//We add a node to the heap, and we resort it to position the new node
void HeapGraph::addNode(GraphNode* newNode) noexcept
{ 
    tree.push_back(newNode);
    sortUp((int)tree.size()-1);
}

/*Recover first*/
//We take the node on the top of the heap, and resort the heap
GraphNode* HeapGraph::recoverFirst() noexcept
{
    if(tree.size() == 0)
        return nullptr;

    GraphNode* aux = tree[0];

    int last = (int) tree.size()-1;

    tree[0] = tree[last];

    tree.erase(tree.begin() + last);

    sortDown(0);

    return aux;

}

/*Contains*/
//Returns true if the heap contains node
bool HeapGraph::contains(GraphNode* node) const noexcept
{
    return std::find(tree.begin(), tree.end(), node) != tree.end();   
}

/*Sort Up*/
//We put the leaf of the heap into the top
void HeapGraph::sortUp( int itemIndex) noexcept
{
    int parentIndex = (itemIndex-1)/2;
    bool search = true;

    while(search){

        int parentFCost = tree[parentIndex]->fCost;
        int childFCost = tree[itemIndex]->fCost;

        if(childFCost < parentFCost)
        {
                //Swap child with parent
                GraphNode* aux = tree[parentIndex];
                tree[parentIndex] = tree[itemIndex];
                tree[itemIndex] = aux;

                itemIndex = parentIndex;
                parentIndex = (itemIndex-1)/2;
        }else{

            search = false;

        }
    }
}

/*Sort Down*/
//We exchange the root for the final leaf, and we climb down the tree to position it
void HeapGraph::sortDown( int itemIndex) noexcept
{
    bool search = true;

    while(search){

        int childIndexLeft = (itemIndex*2) + 1;
        int childIndexRight = (itemIndex*2) + 2;

        GraphNode* left = nullptr;
        GraphNode* right = nullptr;

        if(childIndexLeft > 0 && childIndexLeft < (int)tree.size())
            left = tree[childIndexLeft];
        if(childIndexRight > 0 && childIndexRight < (int)tree.size())
            right = tree[childIndexRight];

        int leftChildFCost = INT_MAX;
        int rightChildFCost = INT_MAX;
        
        if(left)
            leftChildFCost = left->fCost;
        
        if(right)
            rightChildFCost = right->fCost;

        int parentFCost = tree[itemIndex]->fCost;

        int lowestFCost;
        int lowestIndex;

        if(leftChildFCost>rightChildFCost)
        {
            lowestFCost = rightChildFCost;
            lowestIndex = childIndexRight;

        }
        else
        { 
            lowestFCost = leftChildFCost;
            lowestIndex = childIndexLeft;
        }

        if(parentFCost > lowestFCost)
        {
            GraphNode* aux = tree[lowestIndex];
            tree[lowestIndex] = tree[itemIndex];
            tree[itemIndex] = aux;
            itemIndex = lowestIndex; 
        }
        else
            search = false;

    }
}

/*Get Size*/
// We return the size of the heap (its number of nodes)
int HeapGraph::getSize() const noexcept
{
    return (int)tree.size();
}

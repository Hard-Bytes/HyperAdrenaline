#include <algorithm>
#include "graph.hpp"
#include "heapGraph.hpp"

Graph::Graph(GraphicsEngine &graphEngine, GraphicNode *mesh)
: ge(graphEngine), navmesh(mesh)
{
    this->buildGraph();
}

Graph::Graph(GraphicsEngine &graphEngine, std::string filename, bool isObj)
: ge(graphEngine)
{
    if(isObj)
    {
        navmesh = ge.createNode(
            {0,0,0},
            {1,1,1},
            filename,
            false
        );
        this->buildGraph();
    }
}

Graph::~Graph()
{
    for(auto* n : nodes)
        delete n;
    this->nodes.clear();
    navmesh = nullptr;
}

/*Graph: BuildGraph*/
//Build the graph with the power of frendship
void Graph::buildGraph()
{
    //We recover the triangles from the triangle selector   
    std::vector<Triangle> triangles;
    triangles = navmesh->getTriangles();

    //For later use in building the nodes
    Vector3f centerof(0,0,0);

    //We build the Graph
    for(int i=0;i<(int)triangles.size();++i)
    {
        GraphNode *n = new GraphNode(triangles[i],triangles[i].getCenter());
        n->index = i;
        this->nodes.emplace_back(n);
    }

    //Search for the neighbours

    //We assign pointers to the neighbours of the node, and store them in their neightbours vector
    //NOTE: We consider two nodes neighbours if the triangle they contain, shares 2 vertex with another one

    for(auto* node : nodes)
        for(auto* neightbour : nodes)
        {
            if(node == neightbour)  continue;
            if(node->isNeightbour(&(neightbour->triangle)))
               node->neighbours.emplace_back(neightbour);
        }
}

/*Graph: findPathAStar*/
//Return a vector list with the "most" "optimal" path from start to end
std::list<Vector3f> Graph::findPathAStar(Vector3f start, Vector3f end)
{   
    std::list<Vector3f> emptyPath;
    GraphNode *startNode{nullptr}, *endNode{nullptr};

    //If the start or end node is outside the navmesh, the AI doesn't move
    if(!isInTriangle(endNode, end) || !isInTriangle(startNode, start))
        return emptyPath;

    if( std::abs(start.x-end.x)<=0.5 &&
        std::abs(start.z-end.z)<=0.5)
        return emptyPath;
    //We create the two lists (Open is a Heap for better optimization, but it works just as one)
    HeapGraph open(this->nodes.size()); //Here we'll store the nodes that we have to check
    std::list<GraphNode*> closed;       //Here we'll store the nodes that we already checked

    // Initialize fCost, gCost and H of start node
    
    int startHCost = getEstimatedDistance(&start,&end);
    startNode->gCost = 0;
    startNode->hCost = startHCost;
    startNode->fCost = startHCost;
    
    //We add the first node to the open list (or heap)
    open.addNode(startNode);
    
    while(open.getSize()>0)    //<- that >0 is not necesary |:(
    {
        // Take the closest node to the destination and add it to the closed list
        GraphNode *current = open.recoverFirst();
        closed.emplace_back(current);

        //If the current node equals the last node, we reached our destination
        if(current == endNode)  return retracePath(current,end);  

        //We check every neighbour of current
        for(auto* neighbour : current->neighbours)
        {
            //If the neighbour is already in the closed list we ignore it
            if(std::find(closed.begin(),closed.end(), neighbour)!=closed.end()) continue;

            //Calculate new G cost
            int newGcost = 
                current->gCost + 
                getEstimatedDistance(
                    &current->center,&neighbour->center
                );
            
            //Check if the neighbour is already in the open list
            bool inOpen = open.contains(neighbour);
            
            //Check if it's better than the previous
            if(newGcost < neighbour->gCost || !inOpen)
            {
                //Set previous node
                neighbour->previous = current;
                //Update costs
                neighbour->hCost = getEstimatedDistance(&neighbour->center,&end);
                neighbour->gCost = newGcost;
                neighbour->fCost = newGcost + neighbour->hCost;

                //If is not in Open, add it
                if(!inOpen)
                    open.addNode(neighbour);

            }
        }
        current->gCost = INT_MAX;
    }
    
    return emptyPath;
}

/*Graph: isInTriangle*/
//Returns a true if the point is in some triangle of the graph else return false
//The node of the graph that contains the point is returned as a reference
bool Graph::isInTriangle(GraphNode *&gnode, Vector3f pos)
{  
    for(auto* node : nodes)
    {
        if(node->triangle.containPoint(pos))
        {
            gnode = node;
            return true;
        }
    }
    return false;
}

bool Graph::isInTriangle(Vector3f pos)
{
    for(auto* node : nodes)
    {
        if(node->triangle.containPoint(pos))
            return true;
    }

    return false;
}

/*Heuristic Function (h)*/
// For use in the findPathAStar, estimate distance using arbitrary units between the cells aX,aZ to bX,bZ
int Graph::getEstimatedDistance(Vector3f *start, Vector3f *end)
{
    Vector3f vector = *end-*start;
    return sqrt((vector.x*vector.x)+(vector.y*vector.y)+(vector.z*vector.z));    
}

/*Graph: retracePath*/
//Returns a valid path from start to end given a path of closed nodes nodepath
std::list<Vector3f> Graph::retracePath(GraphNode* lastNode, Vector3f end)
{
    std::list<Vector3f> path;
    path.emplace_back(end);
    if(!lastNode) return path;  
    GraphNode *lastcurrent{nullptr}, *current = lastNode;
    //the prevoius center of the node that we are checking
    Vector3f lastChckCenter{0,0,0}; //It is the last center that did NOT collide with the map
    lastChckCenter = current->center;
    lastcurrent=current;
    while(current->previous)
    {
        //check if there are any obstacles in the way, if not
        //skip current node and do not add it to the path
        if( lastChckCenter == current->center)
        {
            lastcurrent = current;
            current = current->reset();
            continue;
        } 
        
        bool mapHindrance = calculateSmoothPath(lastChckCenter, current->center);
        
        //this will be true if there is no clear path
        if(mapHindrance)
        {
            path.emplace_back(lastChckCenter);
            lastChckCenter = lastcurrent->center;
        }
        lastcurrent = current;
        current = current->reset();
    }
    path.reverse();
    return path;
}

bool Graph::calculateSmoothPath(const Vector3f& lastCheckCenter, const Vector3f& currentCenter)
{
    if(useSmoothPaths)
    {
        return ge.checkRayCastCollision(
            lastCheckCenter,
            (currentCenter - lastCheckCenter).normalize(),
            (currentCenter - lastCheckCenter).length(),
            0
        );
    }
    return true;
}

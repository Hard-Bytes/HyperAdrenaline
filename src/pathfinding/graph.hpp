#pragma once
#include <list>
#include <math.h>
#include "../facade/vector3f.hpp"
#include "graphNode.hpp"
#include "../facade/graphicsEngine.hpp"

class Graph
{
    private:
        GraphicsEngine &ge;
    public:

    //Variables
    std::list<GraphNode*> nodes;
    GraphicNode *navmesh;


    //Functions
        //Constructors and destructors
    Graph(GraphicsEngine &ge, std::string filename, bool isObj);    
    Graph(GraphicsEngine &ge, GraphicNode *mesh);       
    ~Graph();

    
    void buildGraph();                                                      //Construir le grafo
    bool isInTriangle(Vector3f position);                               
    bool isInTriangle(GraphNode *&gnode,Vector3f position);
    std::list<Vector3f> findPathAStar(Vector3f start, Vector3f end);        //Buscar path (a*)
    std::list<Vector3f> retracePath(GraphNode* nodepath, Vector3f end);     //Return succesful path as vector3f list
    bool calculateSmoothPath(const Vector3f& lastCheckCenter, const Vector3f& currentCenter);

        //Getters
    int  getEstimatedDistance(Vector3f *start, Vector3f *end);

    // Properties
    bool useSmoothPaths { false };
};


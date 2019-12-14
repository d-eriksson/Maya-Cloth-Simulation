#include <maya/MPointArray.h>
#include <maya/MPoint.h>
#include <maya/MIntArray.h>
#include <MSSNode.cpp>
#include <vector>

class MassSpringSystem{
    public:
    MPointArray& PositionData;
    std::vector<MSSNode> Nodes;
    int connectedNodes;

    MassSpringSystem(MPointArray& P) : PositionData(P){connectedNodes=0;}
    void addNode(MSSNode N);
    void addNode(int center, MIntArray M);
};
void MassSpringSystem::addNode(MSSNode N){
    Nodes.push_back(N);
}
void MassSpringSystem::addNode(int center, MIntArray M){
    MSSNode temp(center);
    for(uint i = 0; i < M.length() ; i++){
        temp.addConnection(M[i], PositionData[center].distanceTo(PositionData[M[i]]));
        
    }
    Nodes.push_back(temp);
    connectedNodes++;
}
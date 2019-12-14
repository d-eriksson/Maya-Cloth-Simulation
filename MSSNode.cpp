#include <maya/MPoint.h>
#include <maya/MItMeshVertex.h>
#include <map>
class MSSNode{
    public:
    int center;
    std::map<int,double> connections;

    MSSNode(int ctr) : center(ctr){}
    void addConnection(int idx, double distance);
};

void MSSNode::addConnection(int idx, double distance){
    connections.insert(std::make_pair(idx,distance));
}

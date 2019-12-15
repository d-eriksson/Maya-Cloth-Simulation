#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MIOStream.h>

class Spring{
    public:
    MSSNode *Node_1;
    MSSNode *Node_2;
    double restLength;
    double k;
    Spring(){};
    Spring( MSSNode *N1,  MSSNode *N2);
    void setForce(unsigned frame);
    void print();

};
Spring::Spring(MSSNode *N1,  MSSNode *N2){
    Node_1 = N1;
    Node_2 = N2;
    restLength = Node_1->positions[0].distanceTo(Node_2->positions[0]);
    k = 1.8;
}
void Spring::setForce(unsigned frame){
    double currentLength = Node_1->positions[frame-1].distanceTo(Node_2->positions[frame-1]);
    double power = k*(currentLength-restLength);
    MVector direction(Node_2->positions[frame-1]-Node_1->positions[frame-1]);
    direction.normalize();
    direction *= power;
    Node_1->Fspring += direction;
    Node_2->Fspring -= direction;

}
void Spring::print(){
    cout << Node_1->index << " => " << Node_2->index << endl;
}
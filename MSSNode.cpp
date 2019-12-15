#include <maya/MPoint.h>
#include <maya/MVector.h>
#include <maya/MPointArray.h>
#include <maya/MIOStream.h>
#include <maya/MDagPath.h>
#include <maya/MFnDagNode.h>
#include <maya/MFnAnimCurve.h>
class MSSNode{
    public:
    MPointArray positions;
    MDagPath path;
    unsigned TotalFrames;
    unsigned index;

    MVector Fspring;
    MVector Vprev;

    double Mass;

    MVector Fg = MVector(0.0,-9.8,0.0);

    MSSNode(){};
    MSSNode(MPoint p, MDagPath dp, unsigned F, unsigned idx);
    void render();
    void print();
    void resetSpringForce();
    void calculatePosition(unsigned frame, MVector wind);

};
MSSNode::MSSNode(MPoint p, MDagPath dp, unsigned F, unsigned idx){
    TotalFrames = F;
    positions.setLength(TotalFrames);
    
    positions[0] = p;
    positions[1] = p;
    path = dp;
    index = idx;
    Fspring = MVector(0.0,0.0,0.0);
    Vprev = MVector(0.0,0.0,0.0);
    Mass = 0.01;
}
void MSSNode::calculatePosition(unsigned frame, MVector wind){
    if(index != 0 && index != 9 ){
        MVector Fnet = Mass * Fg + wind + Fspring; // = Ma
        double invMass = 1/Mass;
        //cout << "( " << frame << " ) "<<Fspring.x << " , "<<Fspring.y << " , "<< Fspring.z << endl;
        MVector V = Vprev + (1.0/24.0)*invMass*Fnet;
        Vprev =V;
        positions[frame] =positions[frame-1] + (1.0/24.0)*V;
        //positions[frame] = 2*positions[frame-1] - positions[frame-2] + Fnet*0.9;
        Fspring *= 0;
    }
    else{
        positions[frame] =positions[frame-1];
    }

}
void MSSNode::print(){
    cout << "(" << TotalFrames << ")Node: " <<  index << endl;
    for(unsigned frame = 1; frame <= TotalFrames; frame++){
        cout << frame << endl;
        //std::cout <<"( " << positions[frame].x << " , " << positions[frame].y << " , " << positions[frame].z << " )" << std::endl;
    }
}
void MSSNode::render(){

    MFnDagNode fn(path);
    MFnAnimCurve curveX;
    MFnAnimCurve curveY;
    MFnAnimCurve curveZ;
    const MObject attrX = fn.attribute("translateX");
    curveX.create(path.transform(), attrX,NULL);
    
    const MObject attrY = fn.attribute("translateY");
    curveY.create(path.transform(), attrY,NULL);

    const MObject attrZ = fn.attribute("translateZ");
    curveZ.create(path.transform(), attrZ,NULL);
    for(unsigned frame = 1; frame <= TotalFrames; frame++){
        MTime tm((double)frame, MTime::kFilm);

        if(MS::kSuccess != curveX.addKeyframe(tm,positions[frame].x)){
            cout << "Error while creating keyframe x" << endl;
        }
        if(MS::kSuccess != curveY.addKeyframe(tm,positions[frame].y)){
            cout << "Error while creating keyframe y" << endl;
        }
        if(MS::kSuccess != curveZ.addKeyframe(tm,positions[frame].z)){
            cout << "Error while creating keyframe z" << endl;
        }
    }
    
}





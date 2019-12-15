#include <vector>
#include <maya/MSimple.h>
#include <maya/MIOStream.h>

#include <maya/MFnDependencyNode.h >
#include <maya/MFnDagNode.h >
#include <maya/MFnAnimCurve.h>
#include <maya/MDGModifier.h >

#include <maya/MSelectionList.h>

#include <maya/MGlobal.h>
#include <maya/MPlug.h >
#include <maya/MObject.h>
#include <maya/MDagPath.h>
#include <maya/MTime.h>
#include <maya/MVector.h>
#include <MSSNode.cpp>
#include <Spring.cpp>
DeclareSimpleCommand( clothSim, "David Eriksson", "2019");
MStatus clothSim::doIt( const MArgList& )
{
    static const unsigned dimX = 10;
    static const unsigned dimY = 10;
    unsigned FRAMES = 24*10;
    MVector wind = MVector(0.01,0.0,0.0);
    
    for(unsigned i = 0 ; i < dimX*dimY; i++){
        MFnDependencyNode fnPolySp;

        MObject	objPolySp = fnPolySp.create("polySphere");
        MFnDagNode fnPolyTrans;

        MObject objPolyTrans = fnPolyTrans.create("transform");
        MFnDagNode fnPolyShape;

        MObject objPolyShp = fnPolyShape.create("mesh", objPolyTrans);
        MDGModifier	dgMod;
        MPlug srcPlug = fnPolySp.findPlug("output");
        MPlug destPlug = fnPolyShape.findPlug("inMesh");
        dgMod.connect(srcPlug, destPlug);
        dgMod.doIt();
    }
    MSelectionList allSpheres;
    MString toMatch("transform*");
    MGlobal::getSelectionListByName(toMatch, allSpheres);
    double xt;
    double yt =3.0;
    double zt;
    MSSNode *Nodes[dimX*dimY];// = new std::vector<MSSNode>();
    for(unsigned s = 0; s < allSpheres.length(); s++ ){
        MObject temp;
        allSpheres.getDependNode(s, temp);
        MFnDependencyNode fn(temp);
        xt =2.0 * (s % dimX);
        zt =2.0* (s / dimY); 
        MPlug translateX = fn.findPlug("translateX");
        MPlug translateY = fn.findPlug("translateY");
        MPlug translateZ = fn.findPlug("translateZ");
        translateX.setValue(xt);
        translateY.setValue(yt);
        translateZ.setValue(zt);
        //double X;
        //translateX.getValue(X);
        //cout << X<< endl;
        MPoint pos(xt,yt,zt);
        MDagPath pth;
        allSpheres.getDagPath(s, pth);
        //MSSNode node(pos,pth, FRAMES,s);
        Nodes[s]= new MSSNode(pos,pth,FRAMES,s);//node;
    }
    
    //Horizontal springs
    std::vector<Spring> springs;
    for(unsigned s = 0; s < allSpheres.length()-1; s++ ){
        if((s+1)% dimX != 0){
            springs.push_back(Spring(Nodes[s], Nodes[s+1]));
        }
    }
    
    //Vertical springs
    for(unsigned s = 0; s < allSpheres.length()-1; s++ ){

        if((s)/ dimY != dimY-1){
            springs.push_back(Spring(Nodes[s], Nodes[s+dimY]));
        }
    }
    // Diagonal springs
    for(unsigned s = 0; s < allSpheres.length()-1; s++ ){
        if(s/dimY != 0 && (s+1)% dimX != 0){ // Bottom left to top right diagonally
            springs.push_back(Spring(Nodes[s], Nodes[s-dimY+1]));
        }
        if(s/dimY != dimY-1 && (s+1)% dimX != 0){ // top left to bottom right diagonally
            springs.push_back(Spring(Nodes[s], Nodes[s+dimY+1]));
        }
    }
    /*int c = 0;
    for(Spring s : springs){
        s.print();
        c++;
    }
    for(unsigned s = 0; s < allSpheres.length(); s++ ){
        Nodes[s]->print();
    }*/
    
    for(unsigned f = 2; f <= FRAMES; f++){

        for(Spring s : springs){
            s.setForce(f);
        }
        for(unsigned s = 0; s < allSpheres.length(); s++ ){
            Nodes[s]->calculatePosition(f,wind);
        }
    }
    
    for(unsigned s = 0; s < allSpheres.length(); s++ ){
            Nodes[s]->render();
    }
    

    
    return MS::kSuccess;
}
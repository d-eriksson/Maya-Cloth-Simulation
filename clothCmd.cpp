#include <string.h>
#include <maya/MIOStream.h>
#include <maya/MStringArray.h>
#include <math.h>
#include <maya/MItGeometry.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItSelectionList.h>
#include <maya/MPxLocatorNode.h> 

#include <maya/MFnNumericAttribute.h>
#include <maya/MFnMatrixAttribute.h>
#include <maya/MFnMatrixData.h>

#include <maya/MFnPlugin.h>
#include <maya/MFnDependencyNode.h>

#include <maya/MTypeId.h> 
#include <maya/MPlug.h>

#include <maya/MDataBlock.h>
#include <maya/MDataHandle.h>
#include <maya/MArrayDataHandle.h>

#include <maya/MPoint.h>
#include <maya/MIntArray.h>
#include <maya/MVector.h>
#include <maya/MMatrix.h>
#include <maya/MPointArray.h>

#include <maya/MDagModifier.h>

#include <maya/MPxGPUDeformer.h>
#include <maya/MGPUDeformerRegistry.h>
#include <maya/MOpenCLInfo.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MFnMesh.h>
#include <clew/clew_cl.h>
#include <vector>
#include <maya/MSimple.h>
#include <maya/MIOStream.h>
#include <maya/MGlobal.h>

#include<MassSpringSystem.cpp>

DeclareSimpleCommand( clothSim, "David Eriksson", "2019");

MStatus clothSim::doIt( const MArgList& )
{
    MSelectionList selection;
    MGlobal::getActiveSelectionList(selection);
    MItSelectionList selection_iter(selection);

    MDagPath selection_DagPath;
    MObject component;
    selection.getDagPath(0, selection_DagPath, component);

    MFnMesh meshFn(selection_DagPath);
    MPointArray positions;
    meshFn.getPoints(positions, MSpace::kWorld);

    MItMeshVertex itr(selection_DagPath, component);
    MIntArray connectedVertices;

    MassSpringSystem MSS(positions);

    for ( ; !itr.isDone(); itr.next() )
    {
        itr.getConnectedVertices(connectedVertices);
        MSS.addNode(itr.index(),connectedVertices);
    }
    cout << MSS.connectedNodes << endl;
    return MS::kSuccess;
}
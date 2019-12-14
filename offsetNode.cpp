//-
// ==========================================================================
// Copyright 2015 Autodesk, Inc.  All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

////////////////////////////////////////////////////////////////////////
// 
// DESCRIPTION:
//
// Produces the dependency graph node "offsetNode".
//
// This plug-in demonstrates how to create a user-defined weighted deformer
// with an associated shape. A deformer is a node which takes any number of
// input geometries, deforms them, and places the output into the output
// geometry attribute. This example plug-in defines a new deformer node
// that offsets vertices according to their CV's weights. The weights are set
// using the set editor or the percent command.
//
// To use this node: 
//	- create a plane or some other object
//	- type: "deformer -type offset" 
//	- a locator is created by the command, and you can use this locator
//	  to control the direction of the offset. The object's CV's will be offset
//	  by the value of the weights of the CV's (the default will be the weight * some constant)
//	  in the direction of the y-vector of the locator 
//	- you can edit the weights using either the component editor or by using
//	  the percent command (eg. percent -v .5 offset1;) 
//
// Use this script to create a simple example with the offset node:
// 
//	loadPlugin offsetNode;
//	polyTorus -r 1 -sr 0.5 -tw 0 -sx 50 -sy 50 -ax 0 1 0 -cuv 1 -ch 1;
//	deformer -type "offset";
//	setKeyframe -v 0 -at rotateZ -t 1 transform1;
//	setKeyframe -v 180 -at rotateZ -t 60 transform1;
//	select -cl;
//
////////////////////////////////////////////////////////////////////////

#include <string.h>
#include <maya/MIOStream.h>
#include <maya/MStringArray.h>
#include <math.h>

#include <maya/MPxDeformerNode.h> 
#include <maya/MItGeometry.h>
#include <maya/MItMeshEdge.h>
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
#include <maya/MVector.h>
#include <maya/MMatrix.h>

#include <maya/MDagModifier.h>

#include <maya/MPxGPUDeformer.h>
#include <maya/MGPUDeformerRegistry.h>
#include <maya/MOpenCLInfo.h>
#include <maya/MViewport2Renderer.h>
#include <maya/MFnMesh.h>
#include <clew/clew_cl.h>
#include <vector>

class offset : public MPxDeformerNode
{
public:
	offset();
	~offset() override;

	static void* creator();
	static MStatus initialize();

	// deformation function
    MStatus deform(MDataBlock& 	block, MItGeometry& iter, const MMatrix& mat, unsigned int multiIndex) override;

	// when the accessory is deleted, this node will clean itself up
	MObject& accessoryAttribute() const override;

	// create accessory nodes when the node is created
	MStatus	accessoryNodeSetup(MDagModifier& cmd) override;

public:
	// local node attributes

	static MObject offsetMatrix; 	// offset center and axis
	
	static MTypeId id;

private:
};

MTypeId offset::id( 0x8000c );

// local attributes
MObject	offset::offsetMatrix;


offset::offset() {}
offset::~offset() {}

void* offset::creator()
{
	return new offset();
}

MStatus offset::initialize()
{
	// local attribute initialization

	MFnMatrixAttribute  mAttr;
	offsetMatrix=mAttr.create( "locateMatrix", "lm");
	mAttr.setStorable(false);
	mAttr.setConnectable(true);

	//  deformation attributes
	addAttribute( offsetMatrix);

	attributeAffects( offset::offsetMatrix, offset::outputGeom );
	

	return MStatus::kSuccess;
}


MStatus
offset::deform( MDataBlock& block,
				MItGeometry& iter,
				const MMatrix& /*m*/,
				unsigned int multiIndex)
//
// Method: deform
//
// Description:   Deform the point with a squash algorithm
//
// Arguments:
//   block		: the datablock of the node
//	 iter		: an iterator for the geometry to be deformed
//   m    		: matrix to transform the point into world space
//	 multiIndex : the index of the geometry that we are deforming
//
//
{
	MStatus returnStatus;
	
	// Envelope data from the base class.
	// The envelope is simply a scale factor.

	MArrayDataHandle input_array = block.inputArrayValue(input);
    input_array.jumpToElement(multiIndex);
    MDataHandle input_data = input_array.inputValue();
	cout << input_data.asString() << endl;

	MDataHandle envData = block.inputValue(envelope, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	float env = envData.asFloat();	

	// Get the matrix which is used to define the direction and scale
	// of the offset.
	//
	MDataHandle matData = block.inputValue(offsetMatrix, &returnStatus );
	if (MS::kSuccess != returnStatus) return returnStatus;
	MMatrix omat = matData.asMatrix();
	MMatrix omatinv = omat.inverse();

	MDataHandle hInputGeom = block.inputValue(inputGeom, &returnStatus);
	if (MS::kSuccess != returnStatus) return returnStatus;
	// iterate through each point in the geometry
	//
	

	MItMeshEdge EdgeIter(inputGeom);
	for(; !EdgeIter.isDone(); EdgeIter.next()){
		MPoint pt0 = EdgeIter.point(0);
		MPoint pt1 = EdgeIter.point(1);
		pt1 *= omatinv;
		cout << "MPoint( " << pt1.x << ", " << pt1.y << ", "<< pt1.z << " )" << endl;
		pt1.y = pt1.y + 1;
		pt1 *= omat;
		EdgeIter.setPoint(pt1 ,1);
	}

	/*for ( ; !iter.isDone(); iter.next()) {
		MPoint pt = iter.position();
		pt *= omatinv;
		
		float weight = weightValue(block,multiIndex,iter.index());

		
		
		pt.y = pt.y + iter.index();

		pt *= omat;
		iter.setPosition(pt);
	}*/
	return returnStatus;
}


/* override */
MObject&
offset::accessoryAttribute() const
//
//	Description:
//	  This method returns a the attribute to which an accessory	
//    shape is connected. If the accessory shape is deleted, the deformer
//	  node will automatically be deleted.
//
//    This method is optional.
//
{
	return offset::offsetMatrix;
}

/* override */
MStatus
offset::accessoryNodeSetup(MDagModifier& cmd)
//
//	Description:
//		This method is called when the deformer is created by the
//		"deformer" command. You can add to the cmds in the MDagModifier
//		cmd in order to hook up any additional nodes that your node needs
//		to operate.
//
//		In this example, we create a locator and attach its matrix attribute
//		to the matrix input on the offset node. The locator is used to
//		set the direction and scale of the random field.
//
//	Description:
//		This method is optional.
//
{
	MStatus result;

	// hook up the accessory node
	//
	MObject objLoc = cmd.createNode(MString("locator"),
									MObject::kNullObj,
									&result);

	if (MS::kSuccess == result) {
		MFnDependencyNode fnLoc(objLoc);
		MString attrName;
		attrName.set("matrix");
		MObject attrMat = fnLoc.attribute(attrName);

		result = cmd.connect(objLoc,attrMat,this->thisMObject(),offset::offsetMatrix);
	}
	return result;
}

// standard initialization procedures
//

MStatus initializePlugin( MObject obj )
{
	MStatus result;
	MFnPlugin plugin( obj, PLUGIN_COMPANY, "3.0", "Any");
	result = plugin.registerNode( "offset", offset::id, offset::creator, 
								  offset::initialize, MPxNode::kDeformerNode );

	MString nodeClassName("offset");
	MString registrantId("mayaPluginExample");
	return result;
}

MStatus uninitializePlugin( MObject obj)
{
	MStatus result;
	MFnPlugin plugin( obj );
	result = plugin.deregisterNode( offset::id );

	MString nodeClassName("offset");
	MString registrantId("mayaPluginExample");

	return result;
}

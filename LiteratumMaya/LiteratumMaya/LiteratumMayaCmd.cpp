//
// Copyright (C) 
// 
// File: LiteratumMayaCmd.cpp
//
// MEL Command: LiteratumMaya
//
// Author: Maya Plug-in Wizard 2.0
//

// Includes everything needed to register a simple MEL command with Maya.
// 
// #include <maya/MSimple.h>
// #include <stdlib.h>
// 
// #include <maya/MFnPlugin.h>
// #include <maya/MTypeId.h>
// 
// 
// #include <maya/MIOStream.h>
// #include <api_macros.h>
// 
// #include <maya/MPlug.h>
// #include <maya/MDataBlock.h>
// #include <maya/MFnNumericAttribute.h>
// #include <maya/MPxThreadedDeviceNode.h>
// 
// 
// #ifdef WIN32
// #define MLL_EXPORT __declspec(dllexport) 
// #else
// #define MLL_EXPORT
// #endif  


#include <maya/MIOStream.h>
#include <maya/MSimple.h>
#include <maya/MTimerMessage.h>
#include <maya/MGlobal.h>

#include "LiteratimTool/LiteratimMain.h"


class LiteratumMaya : public MPxCommand
{
    public:
		LiteratumMaya();
        virtual ~LiteratumMaya();
        static void* creator();
        bool isUndoable() const;
        MStatus doIt(const MArgList&);
        MStatus undoIt();


		MCallbackId tickID;
		static void CallbackTick(float elapsedTime, float lastTime, void *clientData);
		bool IsTicking = false;

		void StopUpdateClock();
		void StartUpdateClock();

		void Tick();

private:


	LiteratimMain MainTool;

public:
	static LiteratumMaya& instance()
	{
		static LiteratumMaya INSTANCE;
		return INSTANCE;
	}
};


// CONSTRUCTOR:
LiteratumMaya::LiteratumMaya()
{
}
// DESTRUCTOR:
LiteratumMaya::~LiteratumMaya()
{
}


// FOR CREATING AN INSTANCE OF THIS COMMAND:
void* LiteratumMaya::creator()
{
	MGlobal::displayInfo("Creating Plugin Object");
	LiteratumMaya::instance().StartUpdateClock();

   return new LiteratumMaya;
}


// MAKE THIS COMMAND NOT UNDOABLE:
bool LiteratumMaya::isUndoable() const
{
   return false;
}


MStatus LiteratumMaya::doIt(const MArgList& args)
{
    MStatus status = MStatus::kSuccess;
    return status;
}

// UNDO THE COMMAND
MStatus LiteratumMaya::undoIt()
{
    MStatus status;
    // undo not implemented
    return status;
}


void LiteratumMaya::StopUpdateClock()
{
	MainTool.Stop();
	MTimerMessage::removeCallback(tickID);
}

void LiteratumMaya::StartUpdateClock()
{
	MGlobal::displayInfo("StartUpdateClock");
	MStatus status2;
	MainTool.Start();
	tickID = MTimerMessage::addTimerCallback(0.1f, &CallbackTick, 0, &status2);
	IsTicking = status2 == MStatus::MStatusCode::kSuccess;
}

void LiteratumMaya::Tick()
{
	MainTool.Tick();
}


void LiteratumMaya::CallbackTick(float elapsedTime, float lastTime, void *clientData)
{
	LiteratumMaya::instance().Tick();
}

// INITIALIZE THE PLUGIN:
MStatus initializePlugin(MObject obj)
{
    MStatus status;
    MFnPlugin plugin(obj, PLUGIN_COMPANY, "9.0", "Any");
    status = plugin.registerCommand("LiteratumStart", LiteratumMaya::creator);
    return status;
}


// UNINITIALIZE THE PLUGIN:
MStatus uninitializePlugin(MObject obj)
{
	LiteratumMaya::instance().StopUpdateClock();
    MStatus status;
    MFnPlugin plugin(obj);
    plugin.deregisterCommand("LiteratumStart");
    return status;
}

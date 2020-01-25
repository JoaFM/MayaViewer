#pragma once

#include "LiteratimTool/LiteratimSceneManager.h"
#include "LiteratimTool/LiteratimNetworking.h"

class LiteratimMain
{
private:

	LiteratimSceneManager SceneManager;
	LiteratimNetworking LitNetwork;
public: 
	void Start();
	void Stop();
	void Tick();

	LiteratimMain();
	~LiteratimMain();
};
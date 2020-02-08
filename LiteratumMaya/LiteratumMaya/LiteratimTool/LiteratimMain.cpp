#include "LiteratimTool/LiteratimMain.h"


void LiteratimMain::Start()
{
	LitNetwork.Connect(&SceneManager);
	SceneManager.SetConnection(&LitNetwork);
}

void LiteratimMain::Stop()
{
	LitNetwork.Disconnect();
	SceneManager.SetConnection(nullptr);
}

void LiteratimMain::Tick()
{
	LitNetwork.Tick();
	SceneManager.Tick();
}

LiteratimMain::LiteratimMain()
{

}

LiteratimMain::~LiteratimMain()
{

}

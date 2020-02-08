#include "LiteratimTool/LiteratimMain.h"


void LiteratimMain::Start()
{
	LitNetwork.Connect(&SceneManager);
	SceneManager.Start(&LitNetwork);
}

void LiteratimMain::Stop()
{
	LitNetwork.Disconnect();
	SceneManager.Stop();
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

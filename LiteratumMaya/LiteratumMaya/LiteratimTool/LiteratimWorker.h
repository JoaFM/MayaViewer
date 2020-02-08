#pragma once
#include "LiteratimMesh.h"

#include <iostream>
#include <thread>
#include <mutex>
#include <list>


class LiteratimSceneManager;
class LiteratimMesh;


class LiteratimWorker
{
public:
	LiteratimWorker();
	~LiteratimWorker();
	
	void Start(LiteratimNetworking* LitNetwork);
	void Stop();
	bool IsReadForNextMesh() const;
	void QueryThisMesh(LiteratimMesh* LitMesh);
	void SendDeferredCommand(std::string& sendCommand);
public:


private:
	//Threads
	std::mutex m_MU_TaskAccses;

	std::thread m_WorkerThread;
	bool m_PendingKill = false;
	void TickThread();
	
	std::mutex m_MU_MessageListAccses;
	std::list<std::string> m_MessageToSend;

	// mesh handeling
	LiteratimMesh* m_ActiveMesh = nullptr;

	// refs
	LiteratimNetworking* m_LitNetwork = nullptr;
};


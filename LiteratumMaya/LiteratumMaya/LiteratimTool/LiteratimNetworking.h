#pragma once

#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include "BufferStack.h"

#include <iostream>
#include <thread>
#include <mutex>

class LiteratimSceneManager;


enum class CurrentState {
	WaitingForPackage,
	WatingForResponceHeader
};

enum class ResponceHeaders {
	ServerCommand = 0,
	Command = 1
};

namespace json
{
	class JSON;
}

struct SendHeader
{
	SendHeader(int _responceType, int _dataSize): 
		responceType(_responceType),
		dataSize(_dataSize)
	{}


	 int responceType = -1;
	 int dataSize = -1;
};

class LiteratimNetworking
{
public:
	LiteratimNetworking();
	~LiteratimNetworking();
	
	bool Connect(LiteratimSceneManager* SceneManager);
	bool Disconnect();
	
	void Tick();

	bool LitSendMessage(std::string message, ResponceHeaders CommandType);
	bool IsConnected() const { return m_connected; };

private:
	std::mutex m_MU_SendMutex;

	char m_buf[4096];
	BufferStack m_CurrentDataStream;

	// server
	SOCKET m_ServerSocket = 0;

	//client state
	CurrentState m_currentState = CurrentState::WatingForResponceHeader;
	void ProcessIncomingData();
	bool m_connected = false;

	const int m_ResponceHeaderSize = 8;
	int m_PackageResponceSize = -1;
	ResponceHeaders m_PackageResponceType;

	void ProcessIncommingCommand(json::JSON* obj);
	bool LitSendBytes(const char* DataToSend, int DataSize);
	LiteratimSceneManager* m_SceneManager;
};


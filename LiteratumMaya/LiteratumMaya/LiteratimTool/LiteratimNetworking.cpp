#include "LiteratimNetworking.h"

#include <maya/MGlobal.h>


#include <stdio.h>
#include <iostream>
#include <string>


#include "External/json.hpp"


LiteratimNetworking::LiteratimNetworking()
{
}


LiteratimNetworking::~LiteratimNetworking()
{

}

bool LiteratimNetworking::Connect()
{
	m_connected = true;

	std::string ipAddress = "127.0.0.1";			// IP Address of the server
	int port = 65432;						// Listening port # on the server

	// Initialize WinSock
	WSAData data;
	WORD ver = MAKEWORD(2, 2);
	int wsResult = WSAStartup(ver, &data);
	if (wsResult != 0)
	{
		MGlobal::displayInfo("Can't start Winsock, Err #" + wsResult );
		m_connected = false;
		return false;
	}

	// Create socket
	 m_ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (m_ServerSocket == INVALID_SOCKET)
	{
		MGlobal::displayInfo("Can't create socket, Err #" + WSAGetLastError());
		m_connected = false;
		WSACleanup();
		return false;
	}

	// Fill in a hint structure
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(port);
	inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

	// Connect to server
	int connResult = connect(m_ServerSocket, (sockaddr*)&hint, sizeof(hint));
	if (connResult == SOCKET_ERROR)
	{
		m_connected = false;
		MGlobal::displayInfo("Can't connect to server, Err #" + WSAGetLastError());
		closesocket(m_ServerSocket);
		WSACleanup();
		return false;
	}

	// set not blocking
	unsigned long NonBlocking = 1;
	int result = ioctlsocket(m_ServerSocket, FIONBIO, &NonBlocking);
	if (result == SOCKET_ERROR)
	{
		int error = WSAGetLastError();
		m_connected = false;
		return false;
	}
	return true;
}

bool LiteratimNetworking::Disconnect()
{
	m_connected = false;
	closesocket(m_ServerSocket);
	WSACleanup();
	return true;
}

void LiteratimNetworking::Tick()
{
	if (!m_connected) {
		MGlobal::displayError("Not Connected to server");
	}
	WSAPOLLFD PollDes = {};

	PollDes.fd = m_ServerSocket;
	PollDes.events = POLLRDNORM | POLLWRNORM;
	PollDes.revents = 0;
	int bytesReceived = 0;

	if (WSAPoll(&PollDes, 1, 0) > 0)	
	{
		if (PollDes.revents  & POLLRDNORM) {
			bytesReceived = recv(m_ServerSocket, m_buf, 4096, 0);
			if (bytesReceived > 0)
			{
				m_CurrentDataStream.Add(m_buf, bytesReceived);
			}
		}
	}
	ProcessIncomingData();


}

void LiteratimNetworking::ProcessIncomingData()
{
	bool dataProcessed = true;
	while (dataProcessed)
	{
		if (m_currentState == CurrentState::WatingForResponceHeader)
		{
			if (m_CurrentDataStream.Num() < m_ResponceHeaderSize) return;

			int HeaderTypeI = -1;
			m_PackageResponceSize = -1;

			std::vector<char> returnData;
			m_CurrentDataStream.GetData(4, returnData);
			memcpy(&HeaderTypeI, returnData.data(), 4);
			m_CurrentDataStream.GetData(4, returnData);
			memcpy(&m_PackageResponceSize, returnData.data(), 4);
			m_PackageResponceType = (ResponceHeaders)HeaderTypeI;
			m_currentState = CurrentState::WaitingForPackage;
			dataProcessed = true;

		}
		else if (m_currentState == CurrentState::WaitingForPackage)
		{
			if (m_CurrentDataStream.Num() < m_PackageResponceSize) return;

			std::vector<char> command;
			m_CurrentDataStream.GetData(m_PackageResponceSize, command);
			dataProcessed = true;

			if (m_PackageResponceType == ResponceHeaders::Command)
			{
				m_currentState = CurrentState::WatingForResponceHeader;
				std::string comstring(command.data(), command.size());
				json::JSON obj = json::JSON::Load(comstring);
				std::string commandStr = obj["Command"].ToString();
				
				if (commandStr == "WhatTypeAreYou")
				{
					if (!LitSendMessage("MAYA", ResponceHeaders::ServerCommand))
					{
						MGlobal::displayError("Failed to send connection response to server");
						Disconnect();
					}
				}
				else
				{
					ProcessIncommingCommand(&obj);
				}
			}
		}
	}
}

void LiteratimNetworking::ProcessIncommingCommand(json::JSON* obj)
{

	std::string CommandStr = obj->ToString();
	MGlobal::displayError(MString("Ignored Command"));
	MGlobal::displayError(MString("CommandStr"));

}

bool LiteratimNetworking::LitSendMessage(std::string message, ResponceHeaders CommandType)
{
	bool SendOK = true;
	SendHeader head((int)(CommandType), (int)message.size());
	SendOK = SendOK && LitSendBytes((char*)(&head), sizeof(head));
	SendOK = SendOK && LitSendBytes(message.data(), (int)message.size());

	if (!SendOK)
	{
		Disconnect();
	}
	return SendOK;
}

bool LiteratimNetworking::LitSendBytes(const char* DataToSend, int DataSize)
{
	int total = 0;        // how many bytes we've sent
	int bytesleft = DataSize; // how many we have left to send
	int sendResult;

	while (total < DataSize) {
		sendResult = send(m_ServerSocket, DataToSend + total, bytesleft, 0);
		if (sendResult == SOCKET_ERROR)
		{

			int err;
			char msgbuf[256];   // for a message up to 255 bytes.
			msgbuf[0] = '\0';    // Microsoft doesn't guarantee this on man page.
			err = WSAGetLastError();
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
				NULL,                // lpsource
				err,                 // message id
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
				msgbuf,              // output buffer
				sizeof(msgbuf),     // size of msgbuf, bytes
				NULL);

			MGlobal::displayError(MString(msgbuf));
			return false;
		}
		if (sendResult == 0)
		{
			return true;
		}
		total += sendResult;
		bytesleft -= sendResult;
	}
	return true;
}

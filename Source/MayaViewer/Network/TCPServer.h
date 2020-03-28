// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "IPv4Endpoint.h"
#include "TCPServer.generated.h"

/**
 * 
 */
class ALiteratiumServer;

UCLASS()
class MAYAVIEWER_API UTCPServer : public UObject
{
	GENERATED_BODY()
	
public:
	static int32 test;
	
	void StartAndClearServer(ALiteratiumServer* MainManager);
	void Disconnect();

	float GetPendingDataFromSocket(TArray<uint8>& DataStream);
	bool Send(const uint8* Data, int32 Count, int32& BytesSent);
	bool IsConnected() ;

	bool IsListeningForConnection();

	void ServerTick();

private:
	static FSocket* m_ListenerSocket;
	static FSocket* m_ConnectionSocket ;
	FIPv4Endpoint RemoteAddressForConnection;

	bool StartTCPReceiver(
		const FString& ListenSocketName,
		const FString& TheIP,
		const int32 ThePort
	);

	FSocket* CreateTCPConnectionListener(
		const FString& YourChosenSocketName,
		const FString& TheIP,
		const int32 ThePort,
		const int32 ReceiveBufferSize = 2 * 1024 * 1024
	);

	//Timer functions, could be threads
	void TCPConnectionListener(); 	//can thread this eventually
		


	//Format String IP4 to number array
	bool FormatIP4ToNumber(const FString& TheIP, uint8(&Out)[4]);

	//Rama's StringFromBinaryArray
	FString StringFromBinaryArray(TArray<uint8>& BinaryArray);



private:
	ALiteratiumServer* m_MainManager = nullptr;

};

// Fill out your copyright notice in the Description page of Project Settings.


#include "TCPServer.h"
#include "Networking.h"
#include "Engine.h"
#include "TCPViewerServer.h"


void UTCPServer::StartAndClearServer(ALiteratiumServer* MainManager)
{
	UE_LOG(LogTemp, Error, TEXT("Starting Server"));
	m_MainManager = MainManager;

	if (!StartTCPReceiver("LiteratiumServer", "127.0.0.1", 65432))
	{
		UE_LOG(LogTemp, Error, TEXT("TCP Socket Listener Creation failed!"));
		return;
	}

}

void UTCPServer::Disconnect()
{
	UE_LOG(LogTemp, Error, TEXT("Stopping Server"));

	if (m_ConnectionSocket)
	{
		m_ConnectionSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(m_ConnectionSocket);
		m_ConnectionSocket = nullptr;
	}
	//Not created?
	if (m_ListenerSocket)
	{
		m_ListenerSocket->Close();
		ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(m_ListenerSocket);
		m_ListenerSocket = nullptr;
	}

}


FSocket* UTCPServer::m_ListenerSocket = nullptr;
FSocket* UTCPServer::m_ConnectionSocket = nullptr;

//Rama's Start TCP Receiver
bool UTCPServer::StartTCPReceiver(
	const FString& ListenSocketName,
	const FString& TheIP,
	const int32 ThePort
) {
	//Rama's CreateTCPConnectionListener
	m_ListenerSocket = CreateTCPConnectionListener(ListenSocketName, TheIP, ThePort);
	//Not created?
	if (!m_ListenerSocket)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("StartTCPReceiver>> Listen socket could not be created! ~> %s %d"), *TheIP, ThePort));
		return false;
	}


	
	//Start the Listener! //thread this eventually
	FTimerHandle LisenTimer;
	//m_MainManager->GetWorldTimerManager().SetTimer(LisenTimer,this, &UTCPServer::TCPConnectionListener, 0.01, true);

	return true;
}



//Rama's Create TCP Connection Listener
FSocket* UTCPServer::CreateTCPConnectionListener(const FString& YourChosenSocketName, const FString& TheIP, const int32 ThePort, const int32 ReceiveBufferSize)
{
	uint8 IP4Nums[4];
	if (!FormatIP4ToNumber(TheIP, IP4Nums))
	{
		UE_LOG(LogTemp, Error, TEXT("Invalid IP! Expecting 4 parts separated by ."));
		return false;
	}

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

	//Create Socket
	FIPv4Endpoint Endpoint(FIPv4Address(IP4Nums[0], IP4Nums[1], IP4Nums[2], IP4Nums[3]), ThePort);
	FSocket* ListenSocket = FTcpSocketBuilder(*YourChosenSocketName)
		.AsReusable()
		.BoundToEndpoint(Endpoint)
		.Listening(8);

	//Set Buffer Size
	int32 NewSize = 0;
	ListenSocket->SetReceiveBufferSize(ReceiveBufferSize, NewSize);

// 	UTCPServer::test++;
// 	UE_LOG(LogTemp, Error, TEXT("Create Listen %d "), UTCPServer::test);

	//Done!
	return ListenSocket;
}


//Rama's TCP Connection Listener
void UTCPServer::TCPConnectionListener()
{
	//~~~~~~~~~~~~~
	if (!IsListeningForConnection())
	{
		return;
	}
	//~~~~~~~~~~~~~

	//Remote address
	TSharedRef<FInternetAddr> RemoteAddress = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	bool Pending;
	ESocketConnectionState tmp = m_ListenerSocket->GetConnectionState();
	// handle incoming connections
	if (m_ListenerSocket->HasPendingConnection(Pending) && Pending)
	{
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		//Already have a Connection? destroy previous
		if (m_ConnectionSocket)
		{
			m_ConnectionSocket->Close();
			ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(m_ConnectionSocket);
		}
		//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

		//New Connection receive!
		m_ConnectionSocket = m_ListenerSocket->Accept(*RemoteAddress, TEXT("RamaTCP Received Socket Connection"));

		if (m_ConnectionSocket != NULL)
		{
			//Global cache of current Remote Address
			RemoteAddressForConnection = FIPv4Endpoint(RemoteAddress);

			//UE_LOG "Accepted Connection! WOOOHOOOO!!!";

			//can thread this too
// 			FTimerHandle ConnectionListen;
// 			m_MainManager->GetWorldTimerManager().SetTimer(ConnectionListen,this,
// 				&UTCPServer::GetPendingDataFromSocket, 0.01, true);
		}
	}
}

//Rama's String From Binary Array
FString UTCPServer::StringFromBinaryArray(TArray<uint8>& BinaryArray)
{
	BinaryArray.Add(0); // Add 0 termination. Even if the string is already 0-terminated, it doesn't change the results.
	// Create a string from a byte array. The string is expected to be 0 terminated (i.e. a byte set to 0).
	// Use UTF8_TO_TCHAR if needed.
	// If you happen to know the data is UTF-16 (USC2) formatted, you do not need any conversion to begin with.
	// Otherwise you might have to write your own conversion algorithm to convert between multilingual UTF-16 planes.
	return FString(ANSI_TO_TCHAR(reinterpret_cast<const char*>(BinaryArray.GetData())));
}

//Rama's TCP Socket Listener
float UTCPServer::GetPendingDataFromSocket(TArray<uint8>& DataStream)
{
	//~~~~~~~~~~~~~
	if (!m_ConnectionSocket) return 0.0f;
	//~~~~~~~~~~~~~

	float totalReceved = 0;
	//Binary Array!
	TArray<uint8> ReceivedData;

	uint32 Size;
	while (IsConnected() && m_ConnectionSocket->HasPendingData(Size))
	{
		ReceivedData.Init(0, FMath::Min(Size, 65507u));
		 
		int32 Read = 0;
		m_ConnectionSocket->Recv(ReceivedData.GetData(), ReceivedData.Num(), Read);

		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Data Read! %d"), ReceivedData.Num()));
		if (ReceivedData.Num())
		{
			totalReceved += (float)ReceivedData.Num();
			uint8* DataCpy = ReceivedData.GetData();

			for (int32 i = 0; i < ReceivedData.Num(); i++)
			{
				DataStream.Add(DataCpy[i]);
			}

		}
	}
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



	//if (!totalReceved) UE_LOG(LogTemp, Error, TEXT("NOTHING"));
	return totalReceved;

}

bool UTCPServer::Send(const uint8* Data, int32 Count, int32& BytesSent)
{
	if (m_ConnectionSocket)
	{
		return m_ConnectionSocket->Send(Data, Count, BytesSent);
	}
	//#TODO react to server lost
	return false;
}

bool UTCPServer::IsConnected() 
{
	if (m_ConnectionSocket == nullptr) return false;
	if (m_ConnectionSocket->GetConnectionState() != SCS_Connected)
	{
		Disconnect();
		return false;
	}
	return true;
}

bool UTCPServer::IsListeningForConnection()
{
	if (m_ListenerSocket == nullptr) return false;
	if (m_ListenerSocket->GetConnectionState() == SCS_ConnectionError)
	{
		Disconnect();
		return false;
	}
	return true;
}

void UTCPServer::ServerTick()
{
	TCPConnectionListener();
}

//Format IP String as Number Parts
bool UTCPServer::FormatIP4ToNumber(const FString& TheIP, uint8(&Out)[4])
{
	//IP Formatting
	TheIP.Replace(TEXT(" "), TEXT(""));

	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//						   IP 4 Parts

	//String Parts
	TArray<FString> Parts;
	TheIP.ParseIntoArray(Parts, TEXT("."), true);
	if (Parts.Num() != 4)
		return false;

	//String to Number Parts
	for (int32 i = 0; i < 4; ++i)
	{
		Out[i] = FCString::Atoi(*Parts[i]);
	}

	return true;
}
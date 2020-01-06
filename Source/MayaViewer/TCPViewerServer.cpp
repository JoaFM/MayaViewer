// Fill out your copyright notice in the Description page of Project Settings.


#include "TCPViewerServer.h"
#include "Runtime\Core\Public\HAL\UnrealMemory.h"
#include "Serialization/JsonSerializer.h"
#include "Dom/JsonObject.h"
#include "SharedPointer.h"
#include "NetInfo/NetCameraInfo.h"
#include "JsonObjectConverter.h"
#include "NetInfo/CommandList.h"
#include "SceneManager/ViewSceneManager.h"

// Sets default values for this component's properties
UTCPViewerServer::UTCPViewerServer()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTCPViewerServer::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTCPViewerServer::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	FString MSG;
	RecvSocketData(SocketToServer,1024, MSG);
	ProcessDataStream();

	
}

UViewSceneManager* UTCPViewerServer::GetViewerScene()
{
	return m_viewerScene;
}

void UTCPViewerServer::Test()
{


}

void UTCPViewerServer::ConnectToServer()
{
	LoadObjects();
	

	SocketToServer = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);

	FIPv4Address ip(127, 0, 0, 1);
	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(65432);

	bool connected = SocketToServer->Connect(*addr);
	UE_LOG(LogTemp, Warning, TEXT("TCPVIEW: %s"), (connected ? TEXT("Connected":TEXT("Fail to Connect"))));

}


void UTCPViewerServer::DissconectToServer()
{
	if (SocketToServer == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("TCPVIEW: %s"),  TEXT("No connection to close"));
		return;
	}
	bool successful = SocketToServer->Close();
	UE_LOG(LogTemp, Warning, TEXT("TCPVIEW: %s"), (successful ? TEXT("Closed Connection":TEXT("Failed to close"))));
}

void UTCPViewerServer::SendTextMessage(FString TextToSend)
{
	if (!SocketToServer) return;
	if (TextToSend.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("No text"));
		return;
	}


	FString serialized = TextToSend + FString("\n");
	TCHAR *serializedChar = serialized.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar);
	int32 sent = 0;

	SendResponceHeader(ResponceHeaders::SetType, size);
	bool successful = SocketToServer->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
	UE_LOG(LogTemp, Warning, TEXT("TCPVIEW: %s"), (successful ? TEXT("Sent":TEXT("Fail to Send"))));
	
}

void UTCPViewerServer::SendResponceHeader(ResponceHeaders ResponceType, int32 ResponceSize)
{
	if (!SocketToServer) return;

	int32 HeaderType = (int32)ResponceType;

	//TODO:: The Sent could fail here!!
	int32 sent = 0;
	SocketToServer->Send((uint8*)(&HeaderType), sizeof(int), sent);
	SocketToServer->Send((uint8*)(&ResponceSize), sizeof(int), sent);
}

bool UTCPViewerServer::RecvSocketData(FSocket *Socket, uint32 DataSize, FString& Msg)
{
	if(!Socket) return false;
	if (!SocketToServer->HasPendingData(DataSize)) return false;

	FArrayReaderPtr Datagram = MakeShareable(new FArrayReader(true));

	Datagram->Init(0,FMath::Min(DataSize, 65507u));

	
	int32 BytesRead = 0;
	if (Socket->Recv(Datagram->GetData(), Datagram->Num(), BytesRead))
	{
		//Data[BytesRead] = '\0';
		//Msg = UTF8_TO_TCHAR(Data);
		if (BytesRead)
		{
			
			uint8* DataCpy = Datagram->GetData();
			
			for (int32 i = 0; i < BytesRead; i++)
			{
				m_CurrentDataStream.Add(DataCpy[i]);
			}

		}
		
		
	}

	return true;
}


void UTCPViewerServer::BeginDestroy()
{
	Super::BeginDestroy();
	DissconectToServer();
}

void UTCPViewerServer::ProcessDataStream()
{
	bool dataProcessed = true;
	if (!m_CurrentDataStream.Num()) return;
	while (dataProcessed)
	{
		if (m_currentState == CurrentState::WatingForResponceHeader)
		{
			if (m_CurrentDataStream.Num() < m_ResponceHeaderSize) return;
			int HeaderTypeI = -1;
			m_PackageResponceSize = -1;

			FMemory::Memcpy(&HeaderTypeI, m_CurrentDataStream.GetData(), 4);
			FMemory::Memcpy(&m_PackageResponceSize, m_CurrentDataStream.GetData() + 4, 4);


			m_PackageResponceType = (ResponceHeaders)HeaderTypeI;

			RemoveRangeOnDataStream(8);
			m_currentState = CurrentState::WaitingForPackage;
			dataProcessed = true;
		}
		else if (m_currentState == CurrentState::WaitingForPackage)
		{
			if (m_CurrentDataStream.Num() < m_PackageResponceSize) return;


			if (m_PackageResponceType == ResponceHeaders::Command)
			{

				TArray<char> Command = RemoveRangeOnDataStream(m_PackageResponceSize);
				Command.Add(0);

				FString CommandString = FString(Command.GetData());
				if (CommandString == "COMMAND_WhatTypeAreYou")
				{
					m_currentState = CurrentState::WatingForResponceHeader;
					SendTextMessage("<COMMAND_WhatTypeAreYou>UE4<COMMAND_WhatTypeAreYou/>");
				}
				dataProcessed = true;

			}
			else if (m_PackageResponceType == ResponceHeaders::Action)
			{
				TArray<char> Command = RemoveRangeOnDataStream(m_PackageResponceSize);
				Command.Add(0);

				FString CommandString = FString(Command.GetData());
				m_currentState = CurrentState::WatingForResponceHeader;

				//Create a json object to store the information from the json string
				//The json reader is used to deserialize the json object later on
				TSharedPtr<FJsonObject> JsonObject = MakeShareable(new FJsonObject());
				TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(CommandString);

				if (FJsonSerializer::Deserialize(JsonReader, JsonObject) && JsonObject.IsValid())
				{
					ProcessCommand(JsonObject);
				}

				dataProcessed = true;

			}
		}
		dataProcessed = false;

	}
	
}

TArray<char> UTCPViewerServer::RemoveRangeOnDataStream(int Size)
{
	TArray<char> Command;
	Command.AddUninitialized(Size);
	FMemory::Memcpy(Command.GetData(), m_CurrentDataStream.GetData(), Size);

	FMemory::Memcpy(m_CurrentDataStream.GetData(), m_CurrentDataStream.GetData()+ Size, m_CurrentDataStream.Num()- Size);
	m_CurrentDataStream.SetNum(m_CurrentDataStream.Num() - Size);
	return Command;
}

void UTCPViewerServer::ProcessCommand(TSharedPtr<FJsonObject> JsonObject)
{
	LoadObjects();
	
	m_CommandList->HandleCommand(JsonObject);
}

void UTCPViewerServer::LoadObjects()
{
	if (m_viewerScene == nullptr)
	{
		m_viewerScene = NewObject<UViewSceneManager>();

	}
	if (!IsValid(m_CommandList))
	{
		m_CommandList = NewObject<UCommandList>();
	}
	m_CommandList->SetViewerScene(m_viewerScene);

}

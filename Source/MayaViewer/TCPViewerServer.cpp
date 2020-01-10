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

#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>

// Sets default values for this component's properties
ALiteratiumServer::ALiteratiumServer()
{

	SetActorTickEnabled(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	// ...
}


// Called when the game starts
void ALiteratiumServer::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

ULiteratumSceneManager* ALiteratiumServer::GetViewerScene()
{
	return m_viewerScene;
}

void ALiteratiumServer::Test()
{

}


void ALiteratiumServer::ConnectToServer()
{
	LoadObjects();

	SocketToServer = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateSocket(NAME_Stream, TEXT("default"), false);

	FIPv4Address ip(127, 0, 0, 1);
	TSharedRef<FInternetAddr> addr = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->CreateInternetAddr();
	addr->SetIp(ip.Value);
	addr->SetPort(65432);

	bool connected = SocketToServer->Connect(*addr);

	GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, TEXT("ConnectToServer"));

	//FMeshObjectMeta _Test;
	//_Test.Min = FVector::OneVector;
	//_Test.Max = -FVector::OneVector;
	//FString TestText;
	//FJsonObjectConverter::UStructToJsonObjectString(_Test, TestText);
	//GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Red, TEXT("ConnectToServer"));
}


void ALiteratiumServer::DissconectToServer()
{
	if (SocketToServer == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("TCPVIEW: %s"),  TEXT("No connection to close"));
		return;
	}
	bool successful = SocketToServer->Close();
	UE_LOG(LogTemp, Warning, TEXT("TCPVIEW: %s"), (successful ? TEXT("Closed Connection":TEXT("Failed to close"))));
	if (successful)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("DissconectToServer Sucsess"));
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("DissconectToServer Failed"));
	}
}

void ALiteratiumServer::SendTextMessage(FString TextToSend, ResponceHeaders ResponceType)
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
	m_uploadAmount += size;

	SendResponceHeader(ResponceType, size);
	bool successful = SocketToServer->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
	UE_LOG(LogTemp, Warning, TEXT("TCPVIEW: %s"), (successful ? TEXT("Sent"):TEXT("Fail to Send")));
	
}

void ALiteratiumServer::SendResponceHeader(ResponceHeaders ResponceType, int32 ResponceSize)
{
	if (!SocketToServer) return;

	int32 HeaderType = (int32)ResponceType;

	//TODO:: The Sent could fail here!!
	int32 sent = 0;
	SocketToServer->Send((uint8*)(&HeaderType), sizeof(int), sent);
	SocketToServer->Send((uint8*)(&ResponceSize), sizeof(int), sent);

	m_uploadAmount += 8;
}

bool ALiteratiumServer::RecvSocketData(FSocket *Socket, uint32 DataSize, FString& Msg)
{
	if(!Socket) return false;
	if (!SocketToServer->HasPendingData(DataSize)) return false;

	FArrayReaderPtr Datagram = MakeShareable(new FArrayReader(true));

	Datagram->Init(0,FMath::Min(DataSize, 65507u));

	
	int32 BytesRead = 0;
	if (Socket->Recv(Datagram->GetData(), Datagram->Num(), BytesRead))
	{
		m_downloadAmount += (float)BytesRead;
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


void ALiteratiumServer::BeginDestroy()
{
	Super::BeginDestroy();
	DissconectToServer();
}

void ALiteratiumServer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	FString MSG;
	RecvSocketData(SocketToServer, 1024, MSG);
	ProcessDataStream();

	if (IsValid(m_CommandList) && m_SceneUpdateTimer > m_SceneUpdateTimerMax)
	{
		m_SceneUpdateTimer = 0;
		m_CommandList->QuerySceneDecription();
	}
	else
	{
		m_SceneUpdateTimer += DeltaSeconds;
	}

	m_downloadAmountTime += DeltaSeconds;
	if (m_downloadAmountTime > 1.0f)
	{
		GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green, FString::Printf(TEXT("down \t %s kbs"),* FString::SanitizeFloat(m_downloadAmount / 1024.0f)));
		GEngine->AddOnScreenDebugMessage(6, 1.f, FColor::Red, FString::Printf(TEXT("Up   \t %s kbs"), *FString::SanitizeFloat(m_uploadAmount / 1024.0f)));

		m_downloadAmountTime = 0;
		m_downloadAmount = m_uploadAmount= 0;
	}
}

void ALiteratiumServer::ProcessDataStream()
{

	bool dataProcessed = true;
	if (!m_CurrentDataStream.Num()) return;
	UE_LOG(LogTemp, Log , TEXT(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Processing <<<<<<<<<<<<<<<<<<<<<<<<<<<"));

	while (dataProcessed)
	{
		dataProcessed = false;

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
			TArray<char> Command = RemoveRangeOnDataStream(m_PackageResponceSize);
			dataProcessed = true;

			Command.Add(0);
			FString CommandString = FString(Command.GetData());
			TSharedPtr<FJsonObject> CommandJson = MakeShareable(new FJsonObject());
			TSharedRef<TJsonReader<>> JsonReader = TJsonReaderFactory<>::Create(CommandString);
			if (!FJsonSerializer::Deserialize(JsonReader, CommandJson) && CommandJson.IsValid())
			{
				// LOG failed to deserilse command json
			}

			if (m_PackageResponceType == ResponceHeaders::Command)
			{
				if (CommandJson->GetStringField("Command") == "WhatTypeAreYou")
				{
					m_currentState = CurrentState::WatingForResponceHeader;
					SendTextMessage("UE4",ResponceHeaders::SetType);
				}

			}
			else if (m_PackageResponceType == ResponceHeaders::Action)
			{

				m_currentState = CurrentState::WatingForResponceHeader;
				FJsonSerializer::Deserialize(JsonReader, CommandJson);
				if (CommandJson.IsValid())
				{
					ProcessCommand(CommandJson);
				}
			}
		}

	}
	
}

TArray<char> ALiteratiumServer::RemoveRangeOnDataStream(int Size)
{
	TArray<char> Command;
	Command.AddUninitialized(Size);
	FMemory::Memcpy(Command.GetData(), m_CurrentDataStream.GetData(), Size);

	FMemory::Memcpy(m_CurrentDataStream.GetData(), m_CurrentDataStream.GetData()+ Size, m_CurrentDataStream.Num()- Size);
	m_CurrentDataStream.SetNum(m_CurrentDataStream.Num() - Size);
	return Command;
}

void ALiteratiumServer::ProcessCommand(TSharedPtr<FJsonObject> JsonObject)
{
	LoadObjects();
	
	m_CommandList->HandleCommand(JsonObject);
}

void ALiteratiumServer::LoadObjects()
{
	if (m_viewerScene == nullptr)
	{
		m_viewerScene = NewObject<ULiteratumSceneManager>();
	}
	if (!IsValid(m_CommandList))
	{
		m_CommandList = NewObject<UCommandList>();
	}
	m_CommandList->Setup(m_viewerScene, this);
	m_viewerScene->Setup(m_CommandList, GetWorld());

}

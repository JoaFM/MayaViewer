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
#include "Network/TCPServer.h"

#include <EngineGlobals.h>
#include <Runtime/Engine/Classes/Engine/Engine.h>
#include "FileManagerGeneric.h"
#include "Materials/Material.h"



//#include "EditorViewportClient.h"

// Sets default values for this component's properties
ALiteratiumServer::ALiteratiumServer()
{
	SetActorTickEnabled(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}


bool ALiteratiumServer::ShouldTickIfViewportsOnly() const
{
	return true;
}

// Called when the game starts
void ALiteratiumServer::BeginPlay()
{
	Super::BeginPlay();
	CloseConnection();
}

ULiteratumSceneManager* ALiteratiumServer::GetViewerScene()
{
	return m_viewerScene;
}


void ALiteratiumServer::MakeSureAServerIsUpAndReady()
{
	LoadObjects();

	if (m_server == nullptr)
	{
		m_server = NewObject<UTCPServer>(this);
	}
	if (IsValid(m_server))
	{
		if (!(m_server->IsConnected() || m_server->IsListeningForConnection()))
		{
			m_server->StartAndClearServer(this);
			DeleteAllActors();
		}
	}
}


void ALiteratiumServer::DirtyAll()
{
	m_CommandList->DirtyContent();
}



bool ALiteratiumServer::IsConnected()
{
	if (IsValid(m_server))
	{
		return m_server->IsConnected();
	}
	return false;
}

void ALiteratiumServer::DeleteAllActors()
{
	if (IsValid(m_viewerScene))
	{
		m_viewerScene->ClearScene();;
	}
}

void ALiteratiumServer::SendTextMessage(FString TextToSend, ResponceHeaders ResponceType)
{
	if (TextToSend.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("No text"));
		return;
	}
	m_actionsSent++;

	FString serialized = TextToSend;
	TCHAR *serializedChar = serialized.GetCharArray().GetData();
	int32 size = FCString::Strlen(serializedChar);
	int32 sent = 0;
	m_uploadAmount += size;

	//#TODO use the server to send the data here
	if (SendResponceHeader(ResponceType, size))
	{
		bool successful = m_server->Send((uint8*)TCHAR_TO_UTF8(serializedChar), size, sent);
		UE_LOG(LogTemp, Log, TEXT("%s Sent %s"), (successful ? TEXT(">>>") : TEXT("ERROR!! : ")), serializedChar);
	}
}

bool ALiteratiumServer::SendResponceHeader(ResponceHeaders ResponceType, int32 ResponceSize)
{
	if (!IsConnected()) {
		return false;
	}

	int32 HeaderType = (int32)ResponceType;

	int32 sent = 0;
	if (!m_server->Send((uint8*)(&HeaderType), sizeof(int), sent)) 
	{
		return false;
	}
	m_uploadAmount += 4;

	if (!m_server->Send((uint8*)(&ResponceSize), sizeof(int), sent)) 
	{
		return false;
	}
	m_uploadAmount += 4;

	return true;
}

bool ALiteratiumServer::RecvSocketData()
{
	if (!IsConnected()) return false;
	m_downloadAmount += m_server->GetPendingDataFromSocket(m_CurrentDataStream);
	return true;
}


void ALiteratiumServer::BeginDestroy()
{
	Super::BeginDestroy();
	DeleteAllActors();
	CloseConnection();
}

void ALiteratiumServer::ServerTick(float DeltaSeconds)
{
	//	#TODO Move the editor camera
	// 	FViewportCameraTransform CamCam;
	// 	CamCam.GetLocation();

	if (m_server)
	{
		m_server->ServerTick();
	}

	MakeSureAServerIsUpAndReady();

	FString MSG;
	//#TODO get this to ask stuff from the server


	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Purple, TEXT("3D App is %s to Viewer"), (IsConnected() ? TEXT("Connected") : TEXT("Not Connected")));
	GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Purple, TEXT("Viewer is%sfor 3D App Connection"), (m_server->IsListeningForConnection() ? TEXT(" ") : TEXT(" Not ")));

	if (!IsConnected())
	{
		return;
	}

	RecvSocketData();
	ProcessDataStream();

	if (IsValid(m_CommandList) && m_SceneUpdateTimer > m_SceneUpdateTimerMax)
	{
		m_SceneUpdateTimer = 0;
	}
	else
	{
		m_SceneUpdateTimer += DeltaSeconds;
	}

	m_downloadAmountTime += DeltaSeconds;
	if (m_downloadAmountTime > 1.0f)
	{
		GEngine->AddOnScreenDebugMessage(5, 1.f, FColor::Green, FString::Printf(TEXT("down ................ %s kbs"), *FString::SanitizeFloat(m_downloadAmount / 1024.0f)));
		GEngine->AddOnScreenDebugMessage(6, 1.f, FColor::Red, FString::Printf(TEXT("Up .................. %s kbs"), *FString::SanitizeFloat(m_uploadAmount / 1024.0f)));
		m_downloadAmount = m_uploadAmount = 0;

		GEngine->AddOnScreenDebugMessage(8, 1.f, FColor::Red, FString::Printf(TEXT("Messages sent ...... %d "), (int32)(m_actionsSent)));
		GEngine->AddOnScreenDebugMessage(9, 1.f, FColor::Red, FString::Printf(TEXT("Commands Processed . %d "), (int32)(m_actionsProcessed)));

		m_actionsSent = m_actionsProcessed = 0;


		m_downloadAmountTime = 0;
	}
}


void ALiteratiumServer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	ServerTick(DeltaSeconds);
}

void ALiteratiumServer::ProcessDataStream()
{

	
	bool dataProcessed = true;
	if (!m_CurrentDataStream.Num()) return;

	while (dataProcessed)
	{
		dataProcessed = false;

		if (m_currentState == CurrentState::WatingForResponceHeader)
		{
			m_actionsProcessed++;
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
			m_actionsProcessed++;
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
			else if (m_PackageResponceType == ResponceHeaders::Command)
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



void ALiteratiumServer::CloseConnection(bool CloseBecauseofError)
{
	m_server->Disconnect();
}

void ALiteratiumServer::CloseConnection()
{
	SendTextMessage("STOP", ResponceHeaders::ServerCommand);
	m_CurrentDataStream.Empty();

	if (IsValid(m_server))
	{
		m_server->Disconnect();
	}
}

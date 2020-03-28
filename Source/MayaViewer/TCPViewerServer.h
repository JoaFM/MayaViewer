// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sockets.h"

#include "Runtime/Networking/Public/Networking.h" 
#include "JsonObject.h"

#include "TCPViewerServer.generated.h"

class UCommandList;
class ULiteratumSceneManager;

class UTCPServer;

UCLASS(Blueprintable, ClassGroup=(Literatum), meta=(BlueprintSpawnableComponent), hideCategories = (Rendering, Replication, Input, Base, Collision, Shape, Transform, Actor, LOD, Cooking))
class MAYAVIEWER_API ALiteratiumServer : public AActor
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	ALiteratiumServer();

	enum class CurrentState {
		WaitingForPackage, 
		WatingForResponceHeader
	};

	enum class ResponceHeaders {
		ServerCommand = 0,
		Command = 1,
	};


protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	float m_SceneUpdateTimer = 0;

	UPROPERTY(Blueprintable, EditAnywhere)
	float m_SceneUpdateTimerMax = 5;


public:	
	// Called every frame

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = TCPViewer)
		 ULiteratumSceneManager* GetViewerScene();

	UFUNCTION(BlueprintCallable)
		void DirtyAll();

	UFUNCTION(BlueprintCallable)
		void CloseConnection();

	UFUNCTION(BlueprintCallable)
		bool IsConnected();

	UFUNCTION(BlueprintCallable)
		void DeleteAllActors();


	void SendTextMessage(FString TextToSend, ResponceHeaders ResponceType);

public:
	virtual void BeginDestroy() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldTickIfViewportsOnly() const override;

protected:
	//Commands 
	UPROPERTY()
		UCommandList* m_CommandList = nullptr;

	UPROPERTY(VisibleAnywhere, Instanced, BlueprintReadOnly)
		ULiteratumSceneManager* m_viewerScene = nullptr;

	void ServerTick(float DeltaSecond);

private:
	UPROPERTY()
		UTCPServer* m_server = nullptr;

	CurrentState m_currentState = CurrentState::WatingForResponceHeader;
	TArray<uint8> m_CurrentDataStream;


	const int m_ResponceHeaderSize = 8;


	void ProcessDataStream();
	TArray<char> RemoveRangeOnDataStream(int Size);

	//Package Sizes
	ResponceHeaders m_PackageResponceType;
	int m_PackageResponceSize;

	//Debug Info
	float m_downloadAmount = 0;
	float m_uploadAmount = 0;
	float m_actionsSent = 0;
	float m_actionsProcessed = 0;
	float m_downloadAmountTime = 0;


	void ProcessCommand(TSharedPtr<FJsonObject> JsonObject);
	void LoadObjects();

	void CloseConnection(bool CloseBecauseofError);

	bool SendResponceHeader(ResponceHeaders ResponceType, int32 ResponceSize);

	bool RecvSocketData();

	void MakeSureAServerIsUpAndReady();
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sockets.h"

#include "Runtime/Networking/Public/Networking.h" 
#include "JsonObject.h"

#include "TCPViewerServer.generated.h"

class UCommandList;
class UViewSceneManager;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAYAVIEWER_API UTCPViewerServer : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTCPViewerServer();

	FSocket* SocketToServer;

	enum class CurrentState {
		WaitingForPackage, 
		WatingForResponceHeader
	};

	enum class ResponceHeaders {
		SetType = 0,
		Command = 1,
		Action = 2,
	};


protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = TCPViewer)
		 UViewSceneManager* GetViewerScene();

	UFUNCTION(BlueprintCallable)
		void Test();

	UFUNCTION(BlueprintCallable)
	void ConnectToServer();

	UFUNCTION(BlueprintCallable)
	void DissconectToServer();

	UFUNCTION(BlueprintCallable)
		void SendTextMessage(FString TextToSend);

	UFUNCTION(BlueprintCallable, BlueprintPure)
		FVector GetCamPos() {return m_ActiveCameraPosition;};



	void SendResponceHeader(ResponceHeaders ResponceType, int32 ResponceSize);

	bool RecvSocketData(FSocket *Socket, uint32 DataSize, FString& Msg);
public:
	virtual void BeginDestroy() override;


private:
	CurrentState m_currentState = CurrentState::WatingForResponceHeader;
	TArray<uint8> m_CurrentDataStream;
	void ProcessDataStream();
	const int m_ResponceHeaderSize = 8;
	TArray<char> RemoveRangeOnDataStream(int Size);

	//Package Sizes
	ResponceHeaders m_PackageResponceType;
	int m_PackageResponceSize;
	FVector m_ActiveCameraPosition;

	//Commands 
	UPROPERTY()
		UCommandList* m_CommandList = nullptr;

	UPROPERTY()
		UViewSceneManager* m_viewerScene = nullptr;


	void ProcessCommand(TSharedPtr<FJsonObject> JsonObject);
	void LoadObjects();
};

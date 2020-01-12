// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SceneManager/ViewSceneManager.h"
#include "LiteratumActorBase.generated.h"


class ULiteratumSceneManager;
class UStaticMeshComponent;

enum class EDirtState {
	Clean = 0, 
	PendingUpdate = 1,
	Dirty = 2
};

UCLASS(Blueprintable)
class MAYAVIEWER_API ALiteratumActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALiteratumActorBase();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;



	UPROPERTY()
		ULiteratumSceneManager* m_SceneManager;

	EDirtState m_IsMeshDirty = EDirtState::Clean;
	EDirtState m_IsTransformDirty = EDirtState::Clean;

public:	


	void SetTransform(FTransform newTransform);
	void Setup(FString ObjectName, ULiteratumSceneManager* SceneManager);
	virtual void OnConnect() {};
	FString GetObjectName() const { return m_ObjectName; }
	void UpdateChangeHash(FSceneObjectHash& ServerHash);

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		FString m_ObjectName;

private:
	
	bool m_TransfromSet = false;

	FTransform m_ActorTransform;
	FSceneObjectHash m_CaschedHash;

	
};

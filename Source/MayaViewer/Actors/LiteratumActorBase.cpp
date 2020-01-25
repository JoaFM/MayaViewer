// Fill out your copyright notice in the Description page of Project Settings.


#include "LiteratumActorBase.h"
#include "DrawDebugHelpers.h"
#include "ConstructorHelpers.h"
#include "SceneManager/ViewSceneManager.h"
#include "JsonObjectConverter.h"





// Sets default values
ALiteratumActorBase::ALiteratumActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	SetActorTickEnabled(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

// Called when the game starts or when spawned
void ALiteratumActorBase::BeginPlay()
{
	Super::BeginPlay();
}


void ALiteratumActorBase::SetTransform(FTransform newTransform)
{
	SetActorTransform(newTransform);
	m_TransfromSet = true;
	m_IsTransformDirty = EDirtState::Clean;
}


void ALiteratumActorBase::Setup(FString ObjectName, ULiteratumSceneManager* SceneManager)
{
	m_ObjectName = ObjectName;
	m_SceneManager = SceneManager;
}

void ALiteratumActorBase::UpdateChangeHash(FSceneObjectHash& ServerHash)
{
	if (m_CaschedHash.h != ServerHash.h)
	{
		UE_LOG(LogTemp, Error, TEXT("%s has A chaned hash for Mesh"), *m_ObjectName);

		m_CaschedHash.h = ServerHash.h;
		m_IsMeshDirty = EDirtState::Dirty;
	}

	if (m_CaschedHash.t != ServerHash.t)
	{
		UE_LOG(LogTemp, Error, TEXT("%s has A changed hash for Transform"), *m_ObjectName);

		m_CaschedHash.t = ServerHash.t;
		m_IsTransformDirty = EDirtState::Dirty;
	}

}


void ALiteratumActorBase::Finish()
{
	m_IsMeshDirty = EDirtState::Clean;

}

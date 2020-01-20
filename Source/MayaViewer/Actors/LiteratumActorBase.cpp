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

bool ALiteratumActorBase::CheckVertBucketSizes(uint32 NumBuckets)
{
	if (m_VertBuckets.Num() != NumBuckets)
	{
		m_VertBuckets.Empty();
		m_VertBuckets.AddDefaulted(NumBuckets);
	}
	return m_VertBuckets.Num() > 0;
}

bool ALiteratumActorBase::CheckTriBucketSizes(uint32 NumBuckets)
{
	if (m_TriBuckets.Num() != NumBuckets)
	{
		m_TriBuckets.Empty();
		m_TriBuckets.AddDefaulted(NumBuckets);
	}
	return m_TriBuckets.Num() > 0;
}


void ALiteratumActorBase::SetMeshVertBucket(TSharedPtr<FJsonObject> MeshVertBucketsJson)
{
	FMeshVertBucket InData;
	FJsonObjectConverter::JsonObjectToUStruct(MeshVertBucketsJson.ToSharedRef(), &InData, 0, 0);

	if (CheckVertBucketSizes(InData.NumBuckets))
	{
		FVertBucket NewBucket;
		NewBucket.HashNum = InData.HashNum;
		NewBucket.VertexPositions = InData.VertexPositions;
		m_VertBuckets[InData.BucketIndex] = NewBucket;
		UE_LOG(LogTemp, Warning, TEXT("Updated Bucket"), *InData.objectName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Bucket Data but with 0 Bucket Count: %s"), *InData.objectName);
	}
}

void ALiteratumActorBase::SetMeshTriBucket(TSharedPtr<FJsonObject> MeshTriBucketsJson)
{
	FMeshTriBucket InData;
	FJsonObjectConverter::JsonObjectToUStruct(MeshTriBucketsJson.ToSharedRef(), &InData, 0, 0);

	if (CheckTriBucketSizes(InData.NumBuckets))
	{
		FTriBucket NewBucket;
		NewBucket.HashNum = InData.HashNum;
		NewBucket.TriIndexs = InData.Tri;
		m_TriBuckets[InData.BucketIndex] = NewBucket;
		UE_LOG(LogTemp, Warning, TEXT("Updated Tri Bucket"), *InData.objectName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Bucket Data but with 0 Bucket Count: %s"), *InData.objectName);
	}
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SceneManager/ViewSceneManager.h"
#include "LiteratumActorBase.generated.h"


class ULiteratumSceneManager;
class UStaticMeshComponent;

USTRUCT()
struct FMeshVertBucket
{
public:
	GENERATED_BODY()

public:
	UPROPERTY()
		FString Command;
	UPROPERTY()
		FString objectName;
	UPROPERTY()
		TArray<FVector> VertexPositions;
	UPROPERTY()
		uint32 HashNum;
	UPROPERTY()
		uint32 Num;
	UPROPERTY()
		uint32 NumBuckets;
	UPROPERTY()
		uint32 BucketIndex;
	UPROPERTY()
		uint32 BucketSize;
};

USTRUCT()
struct FMeshTriBucket
{
public:
	GENERATED_BODY()

public:
	UPROPERTY()
		FString Command;
	UPROPERTY()
		FString objectName;
	UPROPERTY()
		TArray<int32> Tri;
	UPROPERTY()
		uint32 HashNum;
	UPROPERTY()
		uint32 Num;
	UPROPERTY()
		uint32 NumBuckets;
	UPROPERTY()
		uint32 BucketIndex;
	UPROPERTY()
		uint32 BucketSize;
};


USTRUCT()
struct FVertBucket
{
public:
	GENERATED_BODY()

public:
	UPROPERTY()
		TArray<FVector> VertexPositions;
	UPROPERTY()
		uint32 HashNum;
};


USTRUCT()
struct FTriBucket
{
public:
	GENERATED_BODY()

public:
	UPROPERTY()
		TArray<int32> TriIndexs;
	UPROPERTY()
		uint32 HashNum;
};


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

	TArray<FVertBucket> m_VertBuckets;
	TArray<FTriBucket> m_TriBuckets;
	
private:
	bool CheckVertBucketSizes(uint32 NumBuckets);
	bool CheckTriBucketSizes(uint32 NumBuckets);

public:
	void SetMeshVertBucket(TSharedPtr<FJsonObject> MeshVertBucketsJson);
	void SetMeshTriBucket(TSharedPtr<FJsonObject> MeshTriBucketsJson);
};

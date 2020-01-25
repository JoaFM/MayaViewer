// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/LiteratumActorBase.h"
#include "NetInfo/CommandBase.h"
#include "LiterarumMesh.generated.h"

class UProceduralMeshComponent;

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
	UPROPERTY()
		uint32 MatIndex;

	
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
	UPROPERTY()
		int32 MatIndex;
};


USTRUCT()
struct MAYAVIEWER_API FMeshObjectMeta : public FCommandBase
{
	GENERATED_BODY()

public:

	UPROPERTY()
		FString ObjectName;

	UPROPERTY()
		FVector Min;

	UPROPERTY()
		FVector Max;
};


USTRUCT()
struct MAYAVIEWER_API FWholeMeshData : public FCommandBase
{
	GENERATED_BODY()

public:

	UPROPERTY()
		FString ObjectName;

	UPROPERTY()
		TArray <FString> materials;

	UPROPERTY()
		TArray <int32> TriangleMateralStartStop;

	UPROPERTY()
		TArray<int32> materialsTriangles;

	UPROPERTY()
		TArray<FVector> VertexPositions;
};
 

UCLASS(Blueprintable, ClassGroup = (Literatum), meta = (BlueprintSpawnableComponent), hideCategories = (Rendering, Replication, Input, Base, Input, Shape, Actor, HLOD, Cooking, Mobile, Activation, VirtualTexture))
class MAYAVIEWER_API ALiterarumMesh : public ALiteratumActorBase
{
	GENERATED_BODY()
	
public:

	ALiterarumMesh();
	virtual void OnConnect() override;
	virtual void Tick(float DeltaSeconds) override;


	void SetMeshMeta(FMeshObjectMeta& NewMeta);
	void SetWholeMeshData(FWholeMeshData& NewData);




	void SetMeshVertBucket(TSharedPtr<FJsonObject> MeshVertBucketsJson);
	void SetMeshTriBucket(TSharedPtr<FJsonObject> MeshTriBucketsJson);


	virtual void Finish() override;

protected:

	UPROPERTY(VisibleAnywhere)
		UProceduralMeshComponent * m_procMesh;

private:
	FBox m_MeshBounds;
	TArray<FVertBucket> m_VertBuckets;
	TArray<FTriBucket> m_TriBuckets;

	bool CheckVertBucketSizes(uint32 NumBuckets);
	bool CheckTriBucketSizes(uint32 NumBuckets);
};

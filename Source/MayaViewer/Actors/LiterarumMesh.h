// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/LiteratumActorBase.h"
#include "NetInfo/CommandBase.h"
#include "LiterarumMesh.generated.h"

class UProceduralMeshComponent;

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

protected:

	UPROPERTY(VisibleAnywhere)
		UProceduralMeshComponent * m_procMesh;

private:
	FBox m_MeshBounds;

};

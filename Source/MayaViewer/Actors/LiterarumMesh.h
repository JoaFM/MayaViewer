// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/LiteratumActorBase.h"


#include "LiterarumMesh.generated.h"


class UProceduralMeshComponent;

USTRUCT()
struct MAYAVIEWER_API FMeshObjectMeta
{
	GENERATED_BODY()


public:

	UPROPERTY()
		FString Command;

	UPROPERTY()
		FString ObjectName;

	UPROPERTY()
		FVector Min;

	UPROPERTY()
		FVector Max;
};


USTRUCT()
struct MAYAVIEWER_API FWholeMeshData
{
	GENERATED_BODY()


public:

	UPROPERTY()
		FString Command;

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
/**
 * 
 */
UCLASS()
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

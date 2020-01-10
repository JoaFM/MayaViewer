// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Actors/LiteratumActorBase.h"
#include "LiterarumMesh.generated.h"


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

protected:
	UPROPERTY()
		UStaticMeshComponent* m_meshComp;


private:
	FBox m_MeshBounds;


};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NetInfo/NetCameraInfo.h"
#include "JsonObject.h"


#include "Materials/MaterialInstance.h"
#include "Materials/Material.h"


#include "ViewSceneManager.generated.h"


class UCommandList;
class ALiteratumActorBase;
class UCommandList;
class UMaterialInstance;
class UMaterial;

USTRUCT(BlueprintType)
struct  FSceneObjectHash
{
	GENERATED_BODY()

public:
	UPROPERTY()
		FString t;
	
	UPROPERTY()
		FString h;

	UPROPERTY()
		FString c;

};


USTRUCT(BlueprintType)
struct  FSceneDescription : public FCommandBase
{
	GENERATED_BODY()

public:
	UPROPERTY()
		TMap<FString, FSceneObjectHash> SceneObjects;
};

USTRUCT(BlueprintType)
struct  FUpdateObjectTransform : public FCommandBase
{
	GENERATED_BODY()

public:

	UPROPERTY()
		FString objectName;

	UPROPERTY()
		TArray<float> WorldMatrix;
};


/**
 * 
 */
UCLASS(Blueprintable)
class MAYAVIEWER_API ULiteratumSceneManager : public UObject
{
	GENERATED_BODY()
	
public:
	FNetCameraInfo m_CameraInfo;


	void Setup(UCommandList* m_CommandList, UWorld* ParentWorld);

	//Cene
	
	void UpdateSceneObjectTransfrom(TSharedPtr<FJsonObject> InputString);

	FVector MayaToUE4SpacePosition(FVector InPosition) { return FVector(InPosition.X,InPosition.Z,InPosition.Y); }

	UMaterialInstance* GetMaterialInstanceFromContent(FString materials);
	UMaterial* GetMaterialFromContent(FString materials);

	void SetMeshDone(FString UpToDateObjectName);
	void SetMaterialInfo(TSharedPtr<FJsonObject> MaterialsInfoJson);
	void CreateMesh(TSharedPtr<FJsonObject> InputString);
	void DeleteMesh(TSharedPtr<FJsonObject> InputString);

	virtual void BeginDestroy() override;

	void ClearScene();
public:

	UFUNCTION(BlueprintCallable, BlueprintPure)
		const FNetCameraInfo GetActiveCameraInfo() { return m_CameraInfo; };

private:
	friend class UCommandList;
	void SetCamera(FNetCameraInfo NewCamerainfo) { m_CameraInfo = NewCamerainfo; }
	ALiteratumActorBase* GetSceneMeshActor(FString ActorName);


	UPROPERTY()
		UCommandList* m_CommandList;

	UPROPERTY()
		UWorld* m_ParentWorld;

	void UpdateSceneMaterialLibrary();
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		TMap<FString, ALiteratumActorBase*> m_SceneActors;
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TMap<FString, UMaterial*> m_sceneMaterials;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TMap<FString, UMaterialInstance*> m_sceneMaterialInstances;

public:
	void SetMeshBucketVerts(TSharedPtr<FJsonObject> MeshVertBucketsJson);
	void SetMeshBucketTris(TSharedPtr<FJsonObject> MeshVertBucketsJson);
	void SetMeshBucket(TSharedPtr<FJsonObject> MeshBucketsJson);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "JsonObject.h"
#include "TCPViewerServer.h"
#include "CommandList.generated.h"


class ULiteratumSceneManager;

UCLASS(Blueprintable)
class MAYAVIEWER_API UCommandList : public UObject
{
	GENERATED_BODY()
	
public:
	typedef void (UCommandList::*CommandFunctionPtrType)(TSharedPtr<FJsonObject> InputString);


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = ViewerUtils)
		static FVector ConvertLeftHandToUE4VtoV(FVector LefthandedVector);

	void HandleCommand(TSharedPtr<FJsonObject> JsonObject);
	void Setup(ULiteratumSceneManager* NewViewerScene, ALiteratiumServer* server);

public:
	// Actoions OUT
	void RequestObjectTransform(FString ObjectName);
	void RequestObjectMeta(FString ObjectName);
	void RequestObjectWholeData(FString ObjectName);
	void DirtyContent();

private:
	void UpdateActions();

private:
	// Actions IN
	void SetCamera(TSharedPtr<FJsonObject> InputString);
	void SetObjectTransform(TSharedPtr<FJsonObject> InputString);
	void WhatTypeAreYou(TSharedPtr<FJsonObject> InputString);
	void SetMeshDone(TSharedPtr<FJsonObject> commandJsonO);
	void SetMeshBucket(TSharedPtr<FJsonObject> MeshBucketsJson);
	void SetMaterialNames(TSharedPtr<FJsonObject> MaterialsInfoJson);
	void CreateMesh(TSharedPtr<FJsonObject> InputString);
	void DeleteMesh(TSharedPtr<FJsonObject> InputString);

private:

	TMap<FString, CommandFunctionPtrType> Actions;

	UPROPERTY()
		ULiteratumSceneManager* m_ViewerScene;

	UPROPERTY()
		ALiteratiumServer* m_Server;

};

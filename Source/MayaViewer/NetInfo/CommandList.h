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
	void QuerySceneDecription();
	void RequestObjectTransform(FString ObjectName);
	void RequestObjectMeta(FString ObjectName);
	void RequestObjectWholeData(FString ObjectName);

private:
	void UpdateActions();

private:
	// Actions IN
	void SetCamera(TSharedPtr<FJsonObject> InputString);
	void SetObjectMeta(TSharedPtr<FJsonObject> InputString);
	void SetObjectWholeData(TSharedPtr<FJsonObject> InputString);
	void SetSceneDescription(TSharedPtr<FJsonObject> InputString);
	void SetObjectTransform(TSharedPtr<FJsonObject> InputString);
	void WhatTypeAreYou(TSharedPtr<FJsonObject> InputString);
	void SetMeshBucketVerts(TSharedPtr<FJsonObject> MeshVertBucketsJson);
	void SetMeshBucketTris(TSharedPtr<FJsonObject> MeshTriBucketsJson);
	void SetMeshDone(TSharedPtr<FJsonObject> commandJsonO);
	
private:

	TMap<FString, CommandFunctionPtrType> Actions;

	UPROPERTY()
		ULiteratumSceneManager* m_ViewerScene;

	UPROPERTY()
		ALiteratiumServer* m_Server;

};

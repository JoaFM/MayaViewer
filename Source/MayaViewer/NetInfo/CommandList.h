// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "JsonObject.h"
#include "TCPViewerServer.h"
#include "CommandList.generated.h"



/**
 * 
 */

class UViewSceneManager;
UCLASS()
class MAYAVIEWER_API UCommandList : public UObject
{
	GENERATED_BODY()
	
public:
	typedef void (UCommandList::*CommandFunctionPtrType)(TSharedPtr<FJsonObject> InputString);


	void HandleCommand(TSharedPtr<FJsonObject> JsonObject);

	void SetViewerScene(UViewSceneManager* NewViewerScene) { m_ViewerScene = NewViewerScene; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = ViewerUtils)
	static FVector ConvertLeftHandToUE4VtoV(FVector LefthandedVector);

private:
	TMap<FString, CommandFunctionPtrType> Actions;


private:
	void UpdateActions();
	// Actions
	void SetCamera(TSharedPtr<FJsonObject> InputString);

private:
	UPROPERTY()
		UViewSceneManager* m_ViewerScene;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "NetInfo/NetCameraInfo.h"


#include "ViewSceneManager.generated.h"

/**
 * 
 */
UCLASS()
class MAYAVIEWER_API UViewSceneManager : public UObject
{
	GENERATED_BODY()
	
public:
	FNetCameraInfo m_CameraInfo;

public: 

	UFUNCTION(BlueprintCallable, BlueprintPure)
	const FNetCameraInfo GetActiveCameraInfo() { return m_CameraInfo; };

private:
	friend class UCommandList;

	void SetCamera(FNetCameraInfo NewCamerainfo) { m_CameraInfo = NewCamerainfo; }
};

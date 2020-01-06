// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NetInfo/CommandBase.h"
#include "NetCameraInfo.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct MAYAVIEWER_API FNetCameraInfo : public FCommandBase
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadOnly)
		FVector Location;
	

	UPROPERTY(BlueprintReadOnly)
		TArray<float> WorldMatrix;

	UPROPERTY(BlueprintReadOnly)
		FRotator Rotation;

	UPROPERTY(BlueprintReadOnly)
		FVector MayaRotation;


};

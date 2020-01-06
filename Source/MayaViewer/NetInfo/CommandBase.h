// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "CommandBase.generated.h"

/**
 * 
 */
USTRUCT()
struct MAYAVIEWER_API FCommandBase 
{
	GENERATED_BODY()
	

public:
	UPROPERTY()
		FString Command;
};

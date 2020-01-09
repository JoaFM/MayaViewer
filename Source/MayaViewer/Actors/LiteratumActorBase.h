// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiteratumActorBase.generated.h"


class ULiteratumSceneManager;

UCLASS()
class MAYAVIEWER_API ALiteratumActorBase : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALiteratumActorBase();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void SetTransform(FTransform newTransform);
	void Setup(FString ObjectName, ULiteratumSceneManager* SceneManager);

private:
	bool m_TransfromSet = false;
	
	FTransform m_ActorTransform;
	FString m_ObjectName;
	
	UPROPERTY()
		ULiteratumSceneManager* m_SceneManager;
	
	UPROPERTY()
		UStaticMeshComponent* m_meshComp;

};

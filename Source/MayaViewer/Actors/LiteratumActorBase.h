// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LiteratumActorBase.generated.h"


class ULiteratumSceneManager;
class UStaticMeshComponent;


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



	UPROPERTY()
		ULiteratumSceneManager* m_SceneManager;
public:	


	void SetTransform(FTransform newTransform);
	void Setup(FString ObjectName, ULiteratumSceneManager* SceneManager);
	virtual void OnConnect() {};
	FString GetObjectName() const { return m_ObjectName; }

private:
	bool m_TransfromSet = false;
	FTransform m_ActorTransform;
	FString m_ObjectName;
	



};

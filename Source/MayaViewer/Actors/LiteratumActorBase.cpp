// Fill out your copyright notice in the Description page of Project Settings.


#include "LiteratumActorBase.h"
#include "DrawDebugHelpers.h"
#include "ConstructorHelpers.h"

// Sets default values
ALiteratumActorBase::ALiteratumActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	SetActorTickEnabled(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;
}

// Called when the game starts or when spawned
void ALiteratumActorBase::BeginPlay()
{
	Super::BeginPlay();
}


void ALiteratumActorBase::SetTransform(FTransform newTransform)
{
	SetActorTransform(newTransform);
	m_TransfromSet = true;
}


void ALiteratumActorBase::Setup(FString ObjectName, ULiteratumSceneManager* SceneManager)
{
	m_ObjectName = ObjectName;
	m_SceneManager = SceneManager;
}

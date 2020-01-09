// Fill out your copyright notice in the Description page of Project Settings.


#include "LiteratumActorBase.h"
#include "SceneManager/ViewSceneManager.h"
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h"
#include "ConstructorHelpers.h"

// Sets default values
ALiteratumActorBase::ALiteratumActorBase()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	SetActorTickEnabled(true);
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;


	UStaticMeshComponent* createDefaultSubobject = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
	this->m_meshComp = createDefaultSubobject;
	this->m_meshComp->AttachToComponent(GetRootComponent(),FAttachmentTransformRules::KeepRelativeTransform);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("StaticMesh'/Engine/BasicShapes/Sphere.Sphere'"));
	this->m_meshComp->SetStaticMesh(SphereMeshAsset.Object);
}

// Called when the game starts or when spawned
void ALiteratumActorBase::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ALiteratumActorBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DrawDebugSphere(
		GetWorld(),
		m_ActorTransform.GetLocation(),
		50,
		16,
		FColor(255, 0, 0), false, 1.0f
	);

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
	m_SceneManager->RequestObjectTransform(m_ObjectName);
}

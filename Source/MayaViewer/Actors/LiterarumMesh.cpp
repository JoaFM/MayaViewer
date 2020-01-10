// Fill out your copyright notice in the Description page of Project Settings.


#include "LiterarumMesh.h"
#include "ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "SceneManager/ViewSceneManager.h"

ALiterarumMesh::ALiterarumMesh()
{

	UStaticMeshComponent* createDefaultSubobject = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SphereMesh"));
	m_meshComp = createDefaultSubobject;
	SetRootComponent(createDefaultSubobject);
	m_MeshBounds = FBox(FVector::OneVector * -0.5f, FVector::OneVector * 0.5f);
}

void ALiterarumMesh::OnConnect()
{
	m_SceneManager->RequestObjectTransform(GetObjectName());
	m_SceneManager->RequestObjectMeta(GetObjectName());
}

void ALiterarumMesh::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	DrawDebugBox(
		GetWorld(),
		m_MeshBounds.GetCenter(),
		m_MeshBounds.GetExtent(),
		FQuat::Identity,
		FColor(0, 0, 255), false, 1.0f
	);

}

void ALiterarumMesh::SetMeshMeta(FMeshObjectMeta& NewMeta)
{
	m_MeshBounds.Min = NewMeta.Min;
	m_MeshBounds.Max = NewMeta.Max;
}


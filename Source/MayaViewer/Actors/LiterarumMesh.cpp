// Fill out your copyright notice in the Description page of Project Settings.


#include "LiterarumMesh.h"
#include "ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "SceneManager/ViewSceneManager.h"
#include "../Plugins/Runtime/ProceduralMeshComponent/Source/ProceduralMeshComponent/Public/ProceduralMeshComponent.h"


ALiterarumMesh::ALiterarumMesh()
{
	m_procMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = m_procMesh;
	m_procMesh->bUseAsyncCooking = true;
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
	if (m_IsMeshDirty != EDirtState::Clean ||
		m_IsTransformDirty !=EDirtState::Clean
		)
	{
		DrawDebugBox(
			GetWorld(),
			m_MeshBounds.GetCenter(),
			m_MeshBounds.GetExtent(),
			FQuat::Identity,
			FColor(0, 0, 255), false, 1.0f
		);
	}

	if (m_IsMeshDirty == EDirtState::Dirty)
	{
		m_SceneManager->RequestObjectMeta(GetObjectName());
		m_SceneManager->RequestObjectWholeData(GetObjectName());
		m_IsMeshDirty = EDirtState::PendingUpdate;
	}

	if (m_IsTransformDirty == EDirtState::Dirty)
	{
		m_SceneManager->RequestObjectMeta(GetObjectName());
		m_SceneManager->RequestObjectTransform(GetObjectName());
		m_IsTransformDirty = EDirtState::PendingUpdate;
	}

}

void ALiterarumMesh::SetMeshMeta(FMeshObjectMeta& NewMeta)
{
	m_MeshBounds.Min = NewMeta.Min;
	m_MeshBounds.Max = NewMeta.Max;
}

void ALiterarumMesh::SetWholeMeshData(FWholeMeshData& NewData)
{
	for (int32 MatIndex = 0; MatIndex < NewData.materials.Num(); MatIndex++)
	{
		int32 TriEnd = -1;
		int32 TriCount = -1;
		int32 TriStart = NewData.TriangleMateralStartStop[MatIndex];

		if (MatIndex == NewData.materials.Num() - 1)
		{
			TriEnd = NewData.materialsTriangles.Num() ;
		}
		else
		{
			TriEnd = NewData.TriangleMateralStartStop[MatIndex + 1] - TriStart;
		}
		TriCount = TriEnd - TriStart;


		TArray<int32> Triangles;
		Triangles.Reserve(TriCount);

		for (int32 ti = TriStart; ti < TriEnd; ti++)
		{
			Triangles.Add(NewData.materialsTriangles[ti]);
		}

		TArray<FVector> normals;
		TArray<FVector2D> UV0;
		TArray<FProcMeshTangent> tangents;
		TArray<FLinearColor> vertexColors;
		m_procMesh->CreateMeshSection_LinearColor(MatIndex, NewData.VertexPositions, Triangles, normals, UV0, vertexColors, tangents, false);
		
 		UMaterialInstance* MatIns = m_SceneManager->GetMaterialInstanceFromContent(NewData.materials[MatIndex]);
 		if (IsValid(MatIns))
 		{
			m_procMesh->SetMaterial(MatIndex,MatIns);
 
 		}
 		else
 		{
 			UMaterial* Mater = m_SceneManager->GetMaterialFromContent(NewData.materials[MatIndex]);
 			if (IsValid(Mater))
 			{
				m_procMesh->SetMaterial(MatIndex, Mater);
 			}
 		}

		m_IsMeshDirty = EDirtState::Clean;
	}

	UE_LOG(LogTemp, Log, TEXT("Updated The mesh"));
}


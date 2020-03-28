// Fill out your copyright notice in the Description page of Project Settings.


#include "LiterarumMesh.h"
#include "ConstructorHelpers.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "SceneManager/ViewSceneManager.h"
#include "../Plugins/Runtime/ProceduralMeshComponent/Source/ProceduralMeshComponent/Public/ProceduralMeshComponent.h"
#include "JsonObjectConverter.h"


ALiterarumMesh::ALiterarumMesh()
{
	m_procMesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("GeneratedMesh"));
	RootComponent = m_procMesh;
	m_procMesh->bUseAsyncCooking = true;
	m_MeshBounds = FBox(FVector::OneVector * -0.5f, FVector::OneVector * 0.5f);
}

void ALiterarumMesh::OnConnect()
{
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
			m_procMesh->Bounds.GetBox().GetCenter(),
			m_procMesh->Bounds.GetBox().GetExtent(),
			FQuat::Identity,
			FColor(0, 0, 255), false, 1.0f
		);
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


void ALiterarumMesh::SetMeshVertBucket(TSharedPtr<FJsonObject> MeshVertBucketsJson)
{
	FMeshVertBucket InData;
	FJsonObjectConverter::JsonObjectToUStruct(MeshVertBucketsJson.ToSharedRef(), &InData, 0, 0);

	if (CheckVertBucketSizes(InData.NumBuckets))
	{
		FVertBucket NewBucket;
		NewBucket.HashNum = InData.HashNum;
		NewBucket.VertexPositions = InData.VertexPositions;
		m_VertBuckets[InData.BucketIndex] = NewBucket;
		m_IsMeshDirty = EDirtState::Dirty;
		UE_LOG(LogTemp, Warning, TEXT("Updated Bucket"), *InData.objectName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Bucket Data but with 0 Bucket Count: %s"), *InData.objectName);
	}
}

void ALiterarumMesh::SetMeshTriBucket(TSharedPtr<FJsonObject> MeshTriBucketsJson)
{
	FMeshTriBucket InData;
	FJsonObjectConverter::JsonObjectToUStruct(MeshTriBucketsJson.ToSharedRef(), &InData, 0, 0);

	if (CheckTriBucketSizes(InData.NumBuckets))
	{
		FTriBucket NewBucket;
		NewBucket.HashNum = InData.HashNum;
		NewBucket.TriIndexs = InData.Tri;
		NewBucket.MatIndex = InData.MatIndex;
		m_TriBuckets[InData.BucketIndex] = NewBucket;
		m_IsMeshDirty = EDirtState::Dirty;
		UE_LOG(LogTemp, Warning, TEXT("Updated Tri Bucket"), *InData.objectName);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Bucket Data but with 0 Bucket Count: %s"), *InData.objectName);
	}
}

bool ALiterarumMesh::CheckVertBucketSizes(uint32 NumBuckets)
{
	if (m_VertBuckets.Num() != NumBuckets)
	{
		m_VertBuckets.Empty();
		m_VertBuckets.AddDefaulted(NumBuckets);
	}
	return m_VertBuckets.Num() > 0;
}

bool ALiterarumMesh::CheckTriBucketSizes(uint32 NumBuckets)
{
	if (m_TriBuckets.Num() != NumBuckets)
	{
		m_TriBuckets.Empty();
		m_TriBuckets.AddDefaulted(NumBuckets);
	}
	return m_TriBuckets.Num() > 0;
}

bool ALiterarumMesh::CheckMeshBucketSizes(uint32 NumBuckets)
{
	if (m_MeshBuckets.Num() != NumBuckets)
	{
		m_MeshBuckets.Empty();
		m_MeshBuckets.AddDefaulted(NumBuckets);
	}
	return m_MeshBuckets.Num() > 0;
}


void ALiterarumMesh::UpdateMaterial()
{
	for (int MatarialIndex = 0; MatarialIndex < m_Material.Num(); MatarialIndex++)
	{
	
		UMaterialInstance* MatIns = m_SceneManager->GetMaterialInstanceFromContent(m_Material[MatarialIndex]);
		if (IsValid(MatIns))
		{
			m_procMesh->SetMaterial(MatarialIndex, MatIns);
		}
	}
}

void ALiterarumMesh::SetMeshBucket(TSharedPtr<FJsonObject> MeshBucketsJson)
{
	FMeshBucket InData;
	FJsonObjectConverter::JsonObjectToUStruct(MeshBucketsJson.ToSharedRef(), &InData, 0, 0);
	CheckMeshBucketSizes(InData.NumbBuckets);
	m_MeshBuckets[InData.BucketIndex] = InData;
	m_numMaterials = InData.MaterialCount;
	m_IsMeshDirty = EDirtState::Dirty;
}


void ALiterarumMesh::SetMaterialInfo(TSharedPtr<FJsonObject> MaterialInfo)
{
	FMaterialInfo InData;
	FJsonObjectConverter::JsonObjectToUStruct(MaterialInfo.ToSharedRef(), &InData, 0, 0);
	m_Material = InData.MaterialNames;
	UpdateMaterial();
}

void ALiterarumMesh::Finish()
{
	ALiteratumActorBase::Finish();


	// get vert index length
	//int32 vertCount = 0;
	TArray<FVector> VertexPositions;
	TArray<FVector> normals;

	int CurTriI = 0;
	int NumMaterials = 0;
	VertexPositions.Empty();
	normals.Empty();


	TArray<TArray<int32>> Triangles;
	Triangles.AddDefaulted(m_numMaterials);


	for (const FMeshBucket& CurBucket : m_MeshBuckets)
	{
		int CurVertI = 0;
		while (CurVertI < CurBucket.VertPositionsXYZ.Num())
		{
			VertexPositions.Add(FVector(CurBucket.VertPositionsXYZ[CurVertI], CurBucket.VertPositionsXYZ[CurVertI + 2], CurBucket.VertPositionsXYZ[CurVertI + 1]));
			normals.Add(FVector(CurBucket.VertNormalsXYZ[CurVertI], CurBucket.VertNormalsXYZ[CurVertI + 2], CurBucket.VertNormalsXYZ[CurVertI + 1]));
			CurVertI += 3;
		}

		int TriBatchSize =0;
		int CurrentMaterialIndex = -1;
		for (int i = 0; i < CurBucket.TriIndices.Num(); i++)
		{
			TriBatchSize--;

			if (TriBatchSize < 0)
			{
				TriBatchSize = CurBucket.TriIndices[i];
				CurrentMaterialIndex++;
			}
			else
			{
				Triangles[CurrentMaterialIndex].Add(CurBucket.TriIndices[i]);
			}
		}
	}


	

	for (int MatarialIndex = 0; MatarialIndex < Triangles.Num(); MatarialIndex++)
	{
		const TArray<int32>& TriList = Triangles[MatarialIndex];

		TArray<FVector2D> UV0;
		TArray<FProcMeshTangent> tangents;
		TArray<FLinearColor> vertexColors;
		m_procMesh->CreateMeshSection_LinearColor(MatarialIndex, VertexPositions, TriList, normals, UV0, vertexColors, tangents, false);
	}
	UpdateMaterial();
}
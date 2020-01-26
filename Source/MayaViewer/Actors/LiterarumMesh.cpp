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
/*
void ALiterarumMesh::Finish()
{
	ALiteratumActorBase::Finish();


	//////////////////////////////////////////////////////////////////////////
	///                         Materials
	//////////////////////////////////////////////////////////////////////////
	//GetNumMaterials
	int32 matCount = 0;
	for (const FTriBucket& triB : m_TriBuckets)
	{
		matCount = FMath::Max(matCount, triB.MatIndex);
	}
	matCount += 1;


	//////////////////////////////////////////////////////////////////////////
	///                           Verts
	//////////////////////////////////////////////////////////////////////////

	// get vert index length
	int32 vertCount = 0;
	TArray<FVector> VertexPositions;
	{
		for (const FVertBucket& vertB : m_VertBuckets)
		{
			vertCount += vertB.VertexPositions.Num();
		}
		// fill vert list
		VertexPositions.Reserve(vertCount);
		for (const FVertBucket& vertB : m_VertBuckets)
		{
			for (const FVector& P : vertB.VertexPositions)
			{
				VertexPositions.Add(FVector(P.X, P.Z, P.Y));
			}
		}
	}


	for (int32 MaterialIndex = 0; MaterialIndex < matCount; MaterialIndex++)
	{
		//////////////////////////////////////////////////////////////////////////
		///                           TRI
		//////////////////////////////////////////////////////////////////////////

		// get tri list length
		int32 triCount = 0;
		TArray<int32> Triangles;
		{
			for (const FTriBucket& triB : m_TriBuckets)
			{
				if (triB.MatIndex == MaterialIndex)
				{
					triCount += triB.TriIndexs.Num();
				}
			}

			// fill vert list
			Triangles.Reserve(triCount);
			for (const FTriBucket& triB : m_TriBuckets)
			{
				if (triB.MatIndex == MaterialIndex)
				{
					Triangles.Append(triB.TriIndexs);
				}
			}
		}
		//////////////////////////////////////////////////////////////////////////
		///                           Construct
		//////////////////////////////////////////////////////////////////////////

		TArray<FVector> normals;
		TArray<FVector2D> UV0;
		TArray<FProcMeshTangent> tangents;
		TArray<FLinearColor> vertexColors;
		m_procMesh->CreateMeshSection_LinearColor(MaterialIndex, VertexPositions, Triangles, normals, UV0, vertexColors, tangents, false);

		UMaterialInstance* MatIns = m_SceneManager->GetMaterialInstanceFromContent("Red");
		if (IsValid(MatIns))
		{
			m_procMesh->SetMaterial(MaterialIndex, MatIns);
		}
	}


	UE_LOG(LogTemp, Log, TEXT("Updated The mesh"));
}

*/
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

void ALiterarumMesh::SetMeshBucket(TSharedPtr<FJsonObject> MeshBucketsJson)
{
	FMeshBucket InData;
	FJsonObjectConverter::JsonObjectToUStruct(MeshBucketsJson.ToSharedRef(), &InData, 0, 0);
	CheckMeshBucketSizes(InData.NumbBuckets);
	m_MeshBuckets[InData.BucketIndex] = InData;

}

void ALiterarumMesh::Finish()
{
	ALiteratumActorBase::Finish();


	//////////////////////////////////////////////////////////////////////////
	///                         Materials
	//////////////////////////////////////////////////////////////////////////




	// get vert index length
	int32 vertCount = 0;
	TArray<FVector> VertexPositions;
	TArray<int32> Triangles;
	TArray<FVector> normals;

	int CurTriI = 0;

	for (FMeshBucket& CurBucket : m_MeshBuckets)
	{
		int CurVertI = 0;
		while (CurVertI < CurBucket.VertPositionsXYZ.Num())
		{
			VertexPositions.Add(FVector(CurBucket.VertPositionsXYZ[CurVertI], CurBucket.VertPositionsXYZ[CurVertI + 2] , CurBucket.VertPositionsXYZ[CurVertI + 1]));
			normals.Add(FVector(CurBucket.VertNormalsXYZ[CurVertI], CurBucket.VertNormalsXYZ[CurVertI + 2], CurBucket.VertNormalsXYZ[CurVertI +1]));
			CurVertI += 3;
		}

		for (int tri : CurBucket.TriIndices)
		{
			Triangles.Add(tri + CurTriI);
		}
		CurTriI += CurBucket.TriIndices.Num();
	}


	//////////////////////////////////////////////////////////////////////////
	///                           Construct
	//////////////////////////////////////////////////////////////////////////

	TArray<FVector2D> UV0;
	TArray<FProcMeshTangent> tangents;
	TArray<FLinearColor> vertexColors;
	m_procMesh->CreateMeshSection_LinearColor(0, VertexPositions, Triangles, normals, UV0, vertexColors, tangents, false);

	UMaterialInstance* MatIns = m_SceneManager->GetMaterialInstanceFromContent("Red");
	if (IsValid(MatIns))
	{
		m_procMesh->SetMaterial(0, MatIns);
	}
}
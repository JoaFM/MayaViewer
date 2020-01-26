// Fill out your copyright notice in the Description page of Project Settings.


#include "ViewSceneManager.h"
#include "JsonObjectConverter.h"
#include "SharedPointer.h"
#include "NetInfo/CommandList.h"
#include "Engine/World.h"
#include "Actors/LiterarumMesh.h"
#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"


void ULiteratumSceneManager::Setup(UCommandList* CommandList, UWorld* ParentWorld)
{
	m_CommandList = CommandList;
	m_ParentWorld = ParentWorld;

	UpdateSceneMaterialLibrary();


}


void ULiteratumSceneManager::SetObjectMeta(TSharedPtr<FJsonObject> InputJsonObject)
{
	FMeshObjectMeta NewMeta;
	FJsonObjectConverter::JsonObjectToUStruct(InputJsonObject.ToSharedRef(), &NewMeta, 0, 0);
	NewMeta.Min = MayaToUE4SpacePosition(NewMeta.Min);
	NewMeta.Max = MayaToUE4SpacePosition(NewMeta.Max);

	if (m_SceneActors.Contains(NewMeta.ObjectName))
	{
		ALiterarumMesh* LiteratimMesh = (ALiterarumMesh*)m_SceneActors[NewMeta.ObjectName];
		

		if (IsValid(LiteratimMesh))
		{
			LiteratimMesh->SetMeshMeta(NewMeta);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Can not set Meta, Object is not a mesh: %s"), *NewMeta.ObjectName);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Could Not Find Object To Set Meta On: %s"), *NewMeta.ObjectName);

	}
}

void ULiteratumSceneManager::SetObjectWholeData(TSharedPtr<FJsonObject> InputJsonObject)
{
	FWholeMeshData NewMesh;
	FJsonObjectConverter::JsonObjectToUStruct(InputJsonObject.ToSharedRef(), &NewMesh, 0, 0);
	if (m_SceneActors.Contains(NewMesh.ObjectName))
	{
		ALiterarumMesh* LiteratimMesh = (ALiterarumMesh*)m_SceneActors[NewMesh.ObjectName];

		for (int32 i = 0; i < NewMesh.VertexPositions.Num(); i++)
		{
			NewMesh.VertexPositions[i] = FVector(NewMesh.VertexPositions[i].X, NewMesh.VertexPositions[i].Z, NewMesh.VertexPositions[i].Y);
		}

		if (IsValid(LiteratimMesh))
		{
			LiteratimMesh->SetWholeMeshData(NewMesh);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Can not set Data, Object is not a mesh: %s"), *NewMesh.ObjectName);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Could Not Find Object To Set Data On: %s"), *NewMesh.ObjectName);

	}
}



void ULiteratumSceneManager::UpdateSceneDescription(TSharedPtr<FJsonObject> InputJsonObject)
{
	FSceneDescription NetScene;
	FJsonObjectConverter::JsonObjectToUStruct(InputJsonObject.ToSharedRef(), &NetScene, 0, 0);
	int32 objectsCreated = 0;
	int32 objectsRemoved = 0;

	TArray<FString> Keys;
	NetScene.SceneObjects.GetKeys(Keys);
	for (const FString& ObjName : Keys)
	{
		if (m_SceneActors.Contains(ObjName))
		{
			m_SceneActors[ObjName]->UpdateChangeHash(NetScene.SceneObjects[ObjName]);
			continue;
		}
		else
		{
			ALiteratumActorBase* NewObj = m_ParentWorld->SpawnActor<ALiterarumMesh>(ALiterarumMesh::StaticClass(),FTransform::Identity);
			NewObj->Setup(ObjName, this);
			NewObj->OnConnect();
			m_SceneActors.Add(ObjName, NewObj);
			objectsCreated++;
		}
	}

	m_SceneActors.GetKeys(Keys);
	
	// check for orphine objects
	for (const FString& key : Keys)
	{
		if (!NetScene.SceneObjects.Contains(key))
		{
			m_SceneActors[key]->Destroy();
			m_SceneActors.Remove(key);
			objectsRemoved++;
		}
	}
	if (objectsCreated || objectsRemoved)
	{
		UE_LOG(LogTemp, Error, TEXT("SceneDescription Updated Added(%d) removed(%d)"), objectsCreated, objectsRemoved);
	}
}

void ULiteratumSceneManager::UpdateSceneObjectTransfrom(TSharedPtr<FJsonObject> InputJsonObject)
{
	
	FUpdateObjectTransform NewObjectTransformDes;
	FJsonObjectConverter::JsonObjectToUStruct(InputJsonObject.ToSharedRef(), &NewObjectTransformDes, 0, 0);


	FVector Location = FVector(
			NewObjectTransformDes.WorldMatrix[12],
			NewObjectTransformDes.WorldMatrix[14],
			NewObjectTransformDes.WorldMatrix[13]
	);


	FTransform NewTransform;

	NewTransform.SetLocation(Location);
	NewTransform.SetScale3D(NewObjectTransformDes.scale);

	if (m_SceneActors.Contains(NewObjectTransformDes.objectName))
	{
		m_SceneActors[NewObjectTransformDes.objectName]->SetTransform(NewTransform);
	}
}


void ULiteratumSceneManager::RequestObjectTransform(FString ObjectName)
{
	if (IsValid(m_CommandList))
	{
		m_CommandList->RequestObjectTransform(ObjectName);
	}
}

void ULiteratumSceneManager::RequestObjectMeta(FString ObjectName)
{
	if (IsValid(m_CommandList))
	{
		m_CommandList->RequestObjectMeta(ObjectName);
	}
}

void ULiteratumSceneManager::RequestObjectWholeData(FString ObjectName)
{
	if (IsValid(m_CommandList))
	{
		m_CommandList->RequestObjectWholeData(ObjectName);
	}
}

UMaterialInstance* ULiteratumSceneManager::GetMaterialInstanceFromContent(FString materialName)
{
	if (m_sceneMaterialInstances.Contains(materialName))
	{
		return m_sceneMaterialInstances[materialName];
	}
	return nullptr;
}


UMaterial* ULiteratumSceneManager::GetMaterialFromContent(FString materialName)
{
	if (m_sceneMaterials.Contains(materialName))
	{
		return m_sceneMaterials[materialName];
	}
	return nullptr;
}

void ULiteratumSceneManager::SetMeshDone(FString UpToDateObjectName)
{
	ALiteratumActorBase* SceneActor = GetSceneMeshActor(UpToDateObjectName, false);
	if (IsValid(SceneActor))
	{
		SceneActor->Finish();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Could not finish object because it does not exist: %s"), *UpToDateObjectName);
	}
}

void ULiteratumSceneManager::UpdateSceneMaterialLibrary()
{
	TArray<FString> Filenames;
	FPackageName::FindPackagesInDirectory(Filenames, FPaths::ProjectContentDir());

	TArray<FString> assetReferences;
	for (TArray<FString>::TConstIterator FileItem(Filenames); FileItem; ++FileItem)
	{
		assetReferences.Add(FPackageName::FilenameToLongPackageName(*FileItem) + TEXT(".") + FPaths::GetBaseFilename(*FileItem));
	}

	for (FString path : assetReferences)
	{
		UMaterialInstance* MatInstance = LoadObject<UMaterialInstance>(nullptr, *path, *path);
		if (IsValid(MatInstance))
		{
			m_sceneMaterialInstances.Add(MatInstance->GetName(), MatInstance);
			continue;
		}
		else
		{
			UMaterial* Mat = LoadObject<UMaterial>(nullptr, *path, *path);
			if (IsValid(Mat))
			{
				m_sceneMaterials.Add(Mat->GetName(), Mat);
			}
		}
	}
}



ALiteratumActorBase*  ULiteratumSceneManager::GetSceneMeshActor(FString ActorName, bool CreateNew)
{
	if (m_SceneActors.Contains(ActorName))
	{
		return m_SceneActors[ActorName];
	}
	else if (CreateNew)
	{
		ALiteratumActorBase* NewObj = m_ParentWorld->SpawnActor<ALiterarumMesh>(ALiterarumMesh::StaticClass(), FTransform::Identity);
		NewObj->Setup(ActorName, this);
		NewObj->OnConnect();
		m_SceneActors.Add(ActorName, NewObj);
		UE_LOG(LogTemp, Warning, TEXT("Created Mesh actor Object: %s"), *ActorName);
		return NewObj;
	}
	return nullptr;
}


void ULiteratumSceneManager::SetMeshBucketVerts(TSharedPtr<FJsonObject> MeshVertBucketsJson)
{
	FString objname = MeshVertBucketsJson->GetStringField("objectName");
	ALiteratumActorBase* SceneActor = GetSceneMeshActor(objname, true);
		

	if (!IsValid(SceneActor))
	{
		UE_LOG(LogTemp, Error, TEXT("Error Creating object %s"), *objname);
		return;
	}

	ALiterarumMesh* SceneMeshActor = Cast<ALiterarumMesh>(SceneActor);
	if (IsValid(SceneActor))
	{
		SceneMeshActor->SetMeshVertBucket(MeshVertBucketsJson);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Trying to set mesh vertex data on an object that is not a mesh: %s"), *objname);
	}
}

void ULiteratumSceneManager::SetMeshBucketTris(TSharedPtr<FJsonObject> MeshVertBucketsJson)
{
	FString objname = MeshVertBucketsJson->GetStringField("objectName");
	ALiteratumActorBase* SceneActor = GetSceneMeshActor(objname, true);
	if (!IsValid(SceneActor))
	{
		UE_LOG(LogTemp, Error, TEXT("Error Creating object %s"), *objname);
		return;
	}

	ALiterarumMesh* SceneMeshActor = Cast<ALiterarumMesh>(SceneActor);
	if (IsValid(SceneActor))
	{
		SceneMeshActor->SetMeshTriBucket(MeshVertBucketsJson);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Tring to set mesh triangle data on an object that is not a mesh: %s"), *objname);
	}
}

void ULiteratumSceneManager::SetMeshBucket(TSharedPtr<FJsonObject> MeshBucketsJson)
{
	FString ObjectName = MeshBucketsJson->GetStringField("ObjectName");
	ALiteratumActorBase* SceneActor = GetSceneMeshActor(ObjectName, true);


	ALiterarumMesh* SceneMeshActor = Cast<ALiterarumMesh>(SceneActor);
	if (IsValid(SceneActor))
	{
		SceneMeshActor->SetMeshBucket(MeshBucketsJson);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Tring to set mesh  data on an object that is not a mesh: %s"), *ObjectName);
	}
}

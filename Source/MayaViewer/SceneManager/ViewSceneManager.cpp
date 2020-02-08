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




void ULiteratumSceneManager::UpdateSceneObjectTransfrom(TSharedPtr<FJsonObject> InputJsonObject)
{
	
	FUpdateObjectTransform NowTrans;
	FJsonObjectConverter::JsonObjectToUStruct(InputJsonObject.ToSharedRef(), &NowTrans, 0, 0);


	FTransform NewTransform;

	FMatrix mat(
		FPlane(NowTrans.WorldMatrix[0], NowTrans.WorldMatrix[2], NowTrans.WorldMatrix[1], NowTrans.WorldMatrix[3]),
		FPlane(NowTrans.WorldMatrix[8], NowTrans.WorldMatrix[10], NowTrans.WorldMatrix[9], NowTrans.WorldMatrix[11]),
		FPlane(NowTrans.WorldMatrix[4], NowTrans.WorldMatrix[6], NowTrans.WorldMatrix[5], NowTrans.WorldMatrix[7]),
		FPlane(NowTrans.WorldMatrix[12], NowTrans.WorldMatrix[14], NowTrans.WorldMatrix[13], NowTrans.WorldMatrix[15])
	);

	NewTransform.SetFromMatrix(mat);


	if (m_SceneActors.Contains(NowTrans.objectName))
	{
		m_SceneActors[NowTrans.objectName]->SetTransform(NewTransform);
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
	ALiteratumActorBase* SceneActor = GetSceneMeshActor(UpToDateObjectName);
	if (IsValid(SceneActor))
	{
		SceneActor->Finish();
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Could not finish object because it does not exist: %s"), *UpToDateObjectName);
	}
}

void ULiteratumSceneManager::SetMaterialInfo(TSharedPtr<FJsonObject> MaterialsInfoJson)
{

	ALiterarumMesh* SceneActor =Cast<ALiterarumMesh>(GetSceneMeshActor(MaterialsInfoJson->GetStringField("ObjectName")));
	if (IsValid(SceneActor))
	{
		SceneActor->SetMaterialInfo(MaterialsInfoJson);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Could not Set materials object because it does not exist: %s"), *MaterialsInfoJson->GetStringField("objectName"));
	}
}

void ULiteratumSceneManager::CreateMesh(TSharedPtr<FJsonObject> InputString)
{
	FString ActorName = InputString->GetStringField("ObjectName");
	ALiteratumActorBase* NewObj = m_ParentWorld->SpawnActor<ALiterarumMesh>(ALiterarumMesh::StaticClass(), FTransform::Identity);
	NewObj->Setup(ActorName, this);
	NewObj->OnConnect();
	m_SceneActors.Add(ActorName, NewObj);
	UE_LOG(LogTemp, Warning, TEXT("Created Mesh actor Object: %s"), *ActorName);
}

void ULiteratumSceneManager::DeleteMesh(TSharedPtr<FJsonObject> InputString)
{
	ALiteratumActorBase* ActorToDelete = GetSceneMeshActor(InputString->GetStringField("ObjectName"));

	if (IsValid(ActorToDelete))
	{
		m_SceneActors.Remove(InputString->GetStringField("ObjectName"));
		ActorToDelete->Destroy();
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



ALiteratumActorBase*  ULiteratumSceneManager::GetSceneMeshActor(FString ActorName)
{
	if (m_SceneActors.Contains(ActorName))
	{
		return m_SceneActors[ActorName];
	}

	return nullptr;
}


void ULiteratumSceneManager::SetMeshBucketVerts(TSharedPtr<FJsonObject> MeshVertBucketsJson)
{
	FString objname = MeshVertBucketsJson->GetStringField("objectName");
	ALiteratumActorBase* SceneActor = GetSceneMeshActor(objname);
		

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
	ALiteratumActorBase* SceneActor = GetSceneMeshActor(objname);
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
	ALiteratumActorBase* SceneActor = GetSceneMeshActor(ObjectName);


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

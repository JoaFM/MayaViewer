// Fill out your copyright notice in the Description page of Project Settings.


#include "ViewSceneManager.h"
#include "JsonObjectConverter.h"
#include "SharedPointer.h"
#include "NetInfo/CommandList.h"
#include "Engine/World.h"
#include "Actors/LiterarumMesh.h"


void ULiteratumSceneManager::Setup(UCommandList* CommandList, UWorld* ParentWorld)
{
	m_CommandList = CommandList;
	m_ParentWorld = ParentWorld;
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

	UE_LOG(LogTemp, Error, TEXT("SceneDescription Updated Added(%d) removed(%d)"), objectsCreated, objectsRemoved);
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
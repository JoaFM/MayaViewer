// Fill out your copyright notice in the Description page of Project Settings.


#include "ViewSceneManager.h"
#include "JsonObjectConverter.h"
#include "SharedPointer.h"
#include "NetInfo/CommandList.h"
#include "Engine/World.h"


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
			
			ALiteratumActorBase* NewObj = m_ParentWorld->SpawnActor<ALiteratumActorBase>(ALiteratumActorBase::StaticClass(),FTransform::Identity);
			NewObj->Setup(ObjName, this);
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

void ULiteratumSceneManager::Setup(UCommandList* CommandList, UWorld* ParentWorld)
{
	m_CommandList = CommandList;
	m_ParentWorld = ParentWorld;
}

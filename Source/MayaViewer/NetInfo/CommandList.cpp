// Fill out your copyright notice in the Description page of Project Settings.


#include "CommandList.h"
#include "NetCameraInfo.h"
#include "JsonObjectConverter.h"
#include "SceneManager/ViewSceneManager.h"


void UCommandList::UpdateActions()
{
	if (Actions.Num() == 0)
	{
		Actions.Add("SetCamera", &UCommandList::SetCamera);
		Actions.Add("SetObjectTransform", &UCommandList::SetObjectTransform);
		Actions.Add("WhatTypeAreYou", &UCommandList::WhatTypeAreYou);
		Actions.Add("MeshDone", &UCommandList::SetMeshDone);
		Actions.Add("SetMeshBucket", &UCommandList::SetMeshBucket);
		Actions.Add("SetMaterialNames", &UCommandList::SetMaterialNames);
		Actions.Add("CreateMesh", &UCommandList::CreateMesh);
		Actions.Add("DeleteMesh", &UCommandList::DeleteMesh);
		
	}
}


void UCommandList::HandleCommand(TSharedPtr<FJsonObject> JsonObject)
{
	UpdateActions();
	FString j_Command = JsonObject->GetStringField("Command");


	if (Actions.Contains(j_Command))
	{
		UE_LOG(LogTemp, Log, TEXT("Processing command %s"), *j_Command);

		(this->* (Actions[j_Command]))(JsonObject);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed!!  Unknown command %s"), *j_Command);
	}
}


void UCommandList::Setup(ULiteratumSceneManager* NewViewerScene, ALiteratiumServer* server)
{
	m_ViewerScene = NewViewerScene;
	m_Server = server ;
}



FVector UCommandList::ConvertLeftHandToUE4VtoV(FVector LefthandedVector) 
{
	return FVector(LefthandedVector.X, LefthandedVector.Z, LefthandedVector.Y);
}


// --------------- Incoming commands --------------------

void UCommandList::SetCamera(TSharedPtr<FJsonObject> InputString)
{
	if (!IsValid(m_ViewerScene)) return;

	FNetCameraInfo SetCammeraInfo;
	FJsonObjectConverter::JsonObjectToUStruct(InputString.ToSharedRef(), &SetCammeraInfo,0,0);

	FVector Ydir = ConvertLeftHandToUE4VtoV(
		FVector(
			SetCammeraInfo.WorldMatrix[4],
			SetCammeraInfo.WorldMatrix[5],
			SetCammeraInfo.WorldMatrix[6])
	);

	FVector Zdir = ConvertLeftHandToUE4VtoV(
			FVector(
				SetCammeraInfo.WorldMatrix[8],
				SetCammeraInfo.WorldMatrix[9],
				SetCammeraInfo.WorldMatrix[10]
			)
		);

	SetCammeraInfo.Location = ConvertLeftHandToUE4VtoV
	(
		FVector(
			SetCammeraInfo.WorldMatrix[12],
			SetCammeraInfo.WorldMatrix[13],
			SetCammeraInfo.WorldMatrix[14])
	);

	// TODO:  This is really odd. Will confirm when we  Have more objects to test
	SetCammeraInfo.Location.X *= 1;// ?-1
	SetCammeraInfo.Rotation = FRotationMatrix::MakeFromXZ(Zdir, Ydir).Rotator();


	m_ViewerScene->SetCamera(SetCammeraInfo);
;}


void UCommandList::SetObjectTransform(TSharedPtr<FJsonObject> InputString)
{
	m_ViewerScene->UpdateSceneObjectTransfrom(InputString);
}


void UCommandList::WhatTypeAreYou(TSharedPtr<FJsonObject> InputString)
{
	if (!IsValid(m_Server)) return;
	m_Server->SendTextMessage("UE4", ALiteratiumServer::ResponceHeaders::ServerCommand);
	if (m_Server->IsConnected())
	{
		DirtyContent();
	}

}

void UCommandList::SetMeshDone(TSharedPtr<FJsonObject> commandJsonO)
{
	m_ViewerScene->SetMeshDone(commandJsonO->GetStringField("objectName"));
}

void UCommandList::SetMeshBucket(TSharedPtr<FJsonObject> MeshBucketsJson)
{
	m_ViewerScene->SetMeshBucket(MeshBucketsJson);
}

void UCommandList::SetMaterialNames(TSharedPtr<FJsonObject> MaterialsInfoJson)
{
	m_ViewerScene->SetMaterialInfo(MaterialsInfoJson);
}

void UCommandList::CreateMesh(TSharedPtr<FJsonObject> InputString)
{
	m_ViewerScene->CreateMesh(InputString);
}

void UCommandList::DeleteMesh(TSharedPtr<FJsonObject> InputString)
{
	m_ViewerScene->DeleteMesh(InputString);
}

// --------------------  Out going Commands 
void UCommandList::QuerySceneDecription()
{
	if (!IsValid(m_Server)) return;
	m_Server->SendTextMessage("{\"Command\": \"GetSceneDescription\"}", ALiteratiumServer::ResponceHeaders::Command);
}

void UCommandList::RequestObjectTransform(FString ObjectName)
{
	if (!IsValid(m_Server)) return;
	m_Server->SendTextMessage("{\"Command\": \"GetObjectTransform\", \"ObjectName\": \"" + ObjectName + "\"}", ALiteratiumServer::ResponceHeaders::Command);
}

void UCommandList::RequestObjectMeta(FString ObjectName)
{
	if (!IsValid(m_Server)) return;
	m_Server->SendTextMessage("{\"Command\": \"GetObjectMeta\", \"ObjectName\": \"" + ObjectName + "\"}", ALiteratiumServer::ResponceHeaders::Command);

}

void UCommandList::RequestObjectWholeData(FString ObjectName)
{
	if (!IsValid(m_Server)) return;
	m_Server->SendTextMessage("{\"Command\": \"GetObjectWholeData\", \"ObjectName\": \"" + ObjectName + "\"}", ALiteratiumServer::ResponceHeaders::Command);

}

void UCommandList::DirtyContent()
{
	m_Server->SendTextMessage("{\"Command\": \"DirtyContent\"}", ALiteratiumServer::ResponceHeaders::Command);

}

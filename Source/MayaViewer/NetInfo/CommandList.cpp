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
		Actions.Add("SetSceneDescription", &UCommandList::SetSceneDescription);
		Actions.Add("SetObjectTransform", &UCommandList::SetObjectTransform);
		Actions.Add("SetObjectMeta", &UCommandList::SetObjectMeta);
		Actions.Add("SetObjectWholeData", &UCommandList::SetObjectWholeData);
		Actions.Add("WhatTypeAreYou", &UCommandList::WhatTypeAreYou);
		Actions.Add("SetMeshBucketVerts", &UCommandList::SetMeshBucketVerts);
		Actions.Add("SetMeshBucketTris", &UCommandList::SetMeshBucketTris);
		Actions.Add("MeshDone", &UCommandList::SetMeshDone);

		

		
	}
}


void UCommandList::HandleCommand(TSharedPtr<FJsonObject> JsonObject)
{
	UpdateActions();
	FString j_Command = JsonObject->GetStringField("Command");

	if (Actions.Contains(j_Command))
	{
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

void UCommandList::SetObjectMeta(TSharedPtr<FJsonObject> InputString)
{
	m_ViewerScene->SetObjectMeta(InputString);
}



void UCommandList::SetObjectWholeData(TSharedPtr<FJsonObject> InputString)
{
	m_ViewerScene->SetObjectWholeData(InputString);

}

void UCommandList::SetSceneDescription(TSharedPtr<FJsonObject> InputString)
{
	m_ViewerScene->UpdateSceneDescription(InputString);
}


void UCommandList::SetObjectTransform(TSharedPtr<FJsonObject> InputString)
{
	m_ViewerScene->UpdateSceneObjectTransfrom(InputString);
}


void UCommandList::WhatTypeAreYou(TSharedPtr<FJsonObject> InputString)
{
	if (!IsValid(m_Server)) return;
	m_Server->SendTextMessage("UE4", ALiteratiumServer::ResponceHeaders::ServerCommand);

}

void UCommandList::SetMeshBucketVerts(TSharedPtr<FJsonObject> MeshVertBucketsJson)
{
	m_ViewerScene->SetMeshBucketVerts(MeshVertBucketsJson);
}

void UCommandList::SetMeshBucketTris(TSharedPtr<FJsonObject> MeshTriBucketsJson)
{
	m_ViewerScene->SetMeshBucketTris(MeshTriBucketsJson);
}


void UCommandList::SetMeshDone(TSharedPtr<FJsonObject> commandJsonO)
{
	m_ViewerScene->SetMeshDone(commandJsonO->GetStringField("objectName"));
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

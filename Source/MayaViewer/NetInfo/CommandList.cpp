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


FVector UCommandList::ConvertLeftHandToUE4VtoV(FVector LefthandedVector) 
{
	return FVector(LefthandedVector.X, LefthandedVector.Z, LefthandedVector.Y);
}




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

// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"

#include "Components/TextBlock.h"

void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	if (DisplayText)
	{
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	ENetRole RemoteRole = InPawn->GetRemoteRole();
	FString RoleName;
	switch (RemoteRole)
	{
	case ENetRole::ROLE_Authority:
		RoleName=FString("Authority");
		break;
		case ENetRole::ROLE_AutonomousProxy:
		RoleName=FString("Autonomous Proxy");
		break;
		case ENetRole::ROLE_SimulatedProxy:
		RoleName=FString("SimulatedProxy");
		break;
		case ENetRole::ROLE_None:
		RoleName=FString("No Role, Please check why");
		break;
	}

	FString RemoteRoleString=FString::Printf(TEXT("Remote Role: %s" ), *RoleName);
	SetDisplayText(RemoteRoleString);
}

void UOverheadWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

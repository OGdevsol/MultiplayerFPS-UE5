// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:
UPROPERTY(meta =(BindWidget) )
	class UTextBlock* DisplayText;
	void SetDisplayText(FString TextToDisplay); // set as same name as the UWidget text to display player NetRole properties

	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn); // call this function to figure out the net role for player

protected:
	virtual void NativeDestruct() override;
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Blaster/TurningInPlace.h"
#include "BlasterAnimInstance.generated.h"


/**
 * 
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:
	UPROPERTY(BlueprintReadOnly, Category= Character,meta =(AllowPrivateAccess="true"))
	class ABlasterCharacter* BlasterCharacter;
	
	UPROPERTY(BlueprintReadOnly, Category= Movement,meta =(AllowPrivateAccess="true"))
	float Speed;
	UPROPERTY(BlueprintReadOnly, Category= Movement,meta =(AllowPrivateAccess="true"))
	bool bisInAir;
	UPROPERTY(BlueprintReadOnly, Category= Movement,meta =(AllowPrivateAccess="true"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category= Movement,meta =(AllowPrivateAccess="true"))
	bool bWeaponEquipped;

	class AWeapon* EquippedWeapon;
	UPROPERTY(BlueprintReadOnly, Category= Movement,meta =(AllowPrivateAccess="true"))
	bool bIsCrouched;
	UPROPERTY(BlueprintReadOnly, Category= Movement,meta =(AllowPrivateAccess="true"))
	bool bAiming;
	
	UPROPERTY(BlueprintReadOnly, Category= Movement,meta =(AllowPrivateAccess="true"))
	float YawOffset;
	UPROPERTY(BlueprintReadOnly, Category= Movement,meta =(AllowPrivateAccess="true"))
	float Lean;

	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;
	UPROPERTY(BlueprintReadOnly, Category= Movement,meta =(AllowPrivateAccess="true"))
	float AO_YAW;
	UPROPERTY(BlueprintReadOnly, Category= Movement,meta =(AllowPrivateAccess="true"))
	float AO_PITCH;
	UPROPERTY(BlueprintReadOnly, Category= Movement,meta =(AllowPrivateAccess="true"))
	FTransform LeftHandTransform;
	UPROPERTY(BlueprintReadOnly, Category= Movement,meta =(AllowPrivateAccess="true"))
	ETurningInPlace TurningInPlace;
	UPROPERTY(BlueprintReadOnly, Category= Movement,meta =(AllowPrivateAccess="true"))
	FRotator RightHandRotation;
	UPROPERTY(BlueprintReadOnly, Category= Movement,meta =(AllowPrivateAccess="true"))
	bool bLocallyControlled;
};

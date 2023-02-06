// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterAnimInstance.h"

#include "BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UBlasterAnimInstance::NativeInitializeAnimation()
{
	BlasterCharacter=Cast<ABlasterCharacter>(TryGetPawnOwner());
	Super::NativeInitializeAnimation();

	
}

void UBlasterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);
	if (BlasterCharacter==nullptr)
	{
		BlasterCharacter = Cast<ABlasterCharacter>(TryGetPawnOwner());
	}
	if (BlasterCharacter == nullptr) return;
	FVector Velocity=BlasterCharacter->GetVelocity();
	Velocity.Z=0.f;
	Speed=Velocity.Size();
	bisInAir=BlasterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating=BlasterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size()>0.f?true:false;
	bWeaponEquipped = BlasterCharacter->IsWeaponEquipped();
	EquippedWeapon = BlasterCharacter->GetEquippedWeapon();
	bIsCrouched=BlasterCharacter->bIsCrouched;
	bAiming=BlasterCharacter->IsAiming();
	TurningInPlace = BlasterCharacter->GetTurningInPlace();
//Offset yaw for strafing
	FRotator AimRotation = BlasterCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(BlasterCharacter->GetVelocity());
     FRotator DeltaRot	=  UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation,AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation,DeltaRot,DeltaTime,6.f);
	YawOffset = DeltaRotation.Yaw;
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = BlasterCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation,CharacterRotationLastFrame);
	const float Target = Delta.Yaw/DeltaTime;
	const float Interp = FMath::FInterpTo(Lean,Target,DeltaTime,6.f);
	Lean = FMath::Clamp(Interp, -90.f,90.f);
	AO_YAW=BlasterCharacter->GetAO_YAW();
	AO_PITCH = BlasterCharacter->GetAO_PITCH();

	////FABRIK IK In action below....
	///
	if (bWeaponEquipped && EquippedWeapon->GetWeaponMesh() && BlasterCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"),RTS_World); //Convert from world space to bone space
		
		FVector OutPosition;
		FRotator OutRotation;
		BlasterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"),
			LeftHandTransform.GetLocation(),FRotator::ZeroRotator,OutPosition,OutRotation);
	
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));
        FTransform RightHandTranform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("Hand_R"),RTS_World); //Convert from world space to bone space
		RightHandRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTranform.GetLocation(),RightHandTranform.GetLocation() + RightHandTranform.GetLocation() - (BlasterCharacter->GetHitTarget()));

		if (BlasterCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"),RTS_World);
			FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
		}
	
	//	DrawDebugLine(GetWorld(),MuzzleTipTransform.GetLocation(),MuzzleTipTransform.GetLocation()+MuzzleX*1000.f,FColor::Red);
	//	DrawDebugLine(GetWorld(),MuzzleTipTransform.GetLocation(),BlasterCharacter->GetHitTarget(),FColor::Orange);
		
	}
}

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "GameFramework/Character.h"
#include "BlasterCharacter.generated.h"

UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&  OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void Turn(float Value);
	void LookUp(float Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void AimButtonPressed();
	void AimButtonReleased();
	void AimOffset(float DeltaTime);
	
private:
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;
	
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;
	
    UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(AllowPrivateAccess="true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UPROPERTY(VisibleAnywhere)
	UCombatComponent* Combat;

	UFUNCTION(Server,Reliable) // RPC for functions called on clients but executed on server
	void ServerEquipButtonPressed();

	float AO_YAW;
	float AO_PITCH;
	FRotator StartingAimRotation;
	
UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon); //Gets called automatically when a designated variable is replicated // This function can only have an input parameter of the type of the variable being replicated

public:
	 void SetOverlappingWeapon(AWeapon* Weapon); // As soon as the value of overlapping weapon changes, it will replicate
	bool IsWeaponEquipped();

	bool IsAiming();

	FORCEINLINE float GetAO_YAW() const {return AO_YAW;}
	FORCEINLINE float GetAO_PITCH() const {return AO_PITCH;}

	

};

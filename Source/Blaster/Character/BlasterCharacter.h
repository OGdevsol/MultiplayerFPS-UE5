// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
//#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/TurningInPlace.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "GameFramework/Character.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "BlasterCharacter.generated.h"


UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter , public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	ABlasterCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&  OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;
	void PlayFireMontage(bool bAiming);
	//UFUNCTION(NetMulticast,Unreliable)
    //void MultiCastHit();
	virtual void OnRep_ReplicatedMovement() override;
	void Elim();
	

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
	void CalculateAO_Pitch();
	void AimOffset(float DeltaTime);
	void SimProxiesTurn();
	virtual void Jump() override;
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* Damagetype, class AController* InstigatorController, AActor* DamageCauser);
	void UpdateHUDHealth();

	//void OffsetSocketForPlayer();
	
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
	float Interp_AO_YAW;
	FRotator StartingAimRotation;
	
	ETurningInPlace TurningInPlace;
	void TurnInPlace(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* HitReactMontage;
 

	void HideCam(); // Hide camera if player is close
	UPROPERTY(EditAnywhere)
	float CameraThreshold=200.f;

	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;

	float TimeSinceLastMovementReplication;
	float CalculateSpeed();
	//
	// Player Health
	//
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;
	UPROPERTY(ReplicatedUsing = OnRep_Health , VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health();
	
	class ABlasterPlayerController* BlasterPlayerController;
	
UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon); //Gets called automatically when a designated variable is replicated // This function can only have an input parameter of the type of the variable being replicated

public:
	 void SetOverlappingWeapon(AWeapon* Weapon); // As soon as the value of overlapping weapon changes, it will replicate
	bool IsWeaponEquipped();

	bool IsAiming();

	FORCEINLINE float GetAO_YAW() const {return AO_YAW;}
	FORCEINLINE float GetAO_PITCH() const {return AO_PITCH;}
	AWeapon* GetEquippedWeapon();
	
	FORCEINLINE ETurningInPlace GetTurningInPlace() const {return TurningInPlace;}
	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent* GetFollowCamera(){return FollowCamera;}

	FORCEINLINE bool ShouldRotateRootBone() const {return bRotateRootBone;}
	

};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"
#define TRACE_LENGTH 80000.f;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	friend class ABlasterCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>&  OutLifetimeProps) const override;
	

	void EquipWeapon(class AWeapon* WeaponToEquip);

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);
	
    UFUNCTION()
	void OnRep_EquippedWeapon();
	void FireButtonPressed(bool bPressed);

	UFUNCTION(Server,Reliable)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget);
	UFUNCTION(NetMulticast,Reliable)
	void MultiCastFire(const FVector_NetQuantize& TraceHitTarget);

	void GetViewport(FVector2D ViewportSize);
	void TraceUnderCrosshairs(FHitResult& HitResult);

	void SetHUDCrosshairs(float DeltaTime);

	
	
	
private:
	class ABlasterCharacter* Character;
	class ABlasterPlayerController* Controller;
	class ABlasterHUD* HUD;

	UPROPERTY(ReplicatedUsing=OnRep_EquippedWeapon)
	class AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	bool bFirePressed;

	FVector StartPosition;
	FVector EndPosition;

	//HUD
	float CrosshairVelocityFactor;
	float CrosshairAirVelocityFactor;
    float CrosshairAimFactor;
	float CrosshairShootingFactor;
	FHUDPackage HUDPackage;

	
	FVector HitTarget;

//	FVector HitTarget;


	//Aiming and FOV
	float DefaultFOV; //When not aiming. Set to cam's base FOV in BeginPlay
	UPROPERTY(EditAnywhere, Category=Combat)
	float ZoomedFOV =30.f; //When aiming. Inifialized at 30.f, can be modified anywhere
	UPROPERTY(EditAnywhere, Category=Combat)
	float ZoomInterpSpeed=20.f;

	void InterpFOV(float DeltaTime);

	float CurrentFOV;
public:
};

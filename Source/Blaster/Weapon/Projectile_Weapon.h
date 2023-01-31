// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Projectile_Weapon.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API AProjectile_Weapon : public AWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;
	
	
private:

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;
	

	
};

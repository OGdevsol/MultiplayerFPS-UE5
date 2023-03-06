// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "NiagaraComponent.h"
#include "ProjectileRocket.generated.h"


/**
 * 
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
public:
	AProjectileRocket();
	virtual void Destroyed() override;
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit ) override;
	virtual void BeginPlay() override;
	
	void DestroyTimerFinisher();
	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoop;
	UPROPERTY()
	UAudioComponent* ProjectileloopComponent;
	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;
	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;
private:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* RocketMesh;
	
	
};

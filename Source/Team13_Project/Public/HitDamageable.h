// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HitDamageable.generated.h"


UINTERFACE(MinimalAPI, Blueprintable)
class UHitDamageable : public UInterface
{
	GENERATED_BODY()
};

class TEAM13_PROJECT_API IHitDamageable
{
	GENERATED_BODY()

	
public:
	
	virtual float GetCurrentHealth() const = 0;
	virtual float GetMaxHealth() const = 0;
	virtual void SetCurrentHealth(float NewValue) = 0;

	virtual int32 GetLevel() const = 0;
	virtual float GetSizeScale() const = 0;
	virtual float GetMaxSpeed() const = 0;
	virtual float GetCurrentSpeed() const = 0;

	virtual bool IsDead() const = 0;
	virtual void OnDead() = 0;
	/**
	* 레그돌로 변환하고 충격 주기
	* @ Impulse 충격 방향
	*/
	virtual void EnableRagdollAndImpulse(const FVector& Impulse) = 0;




};

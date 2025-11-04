// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "CombatFeedbackSettings.generated.h"

class USoundBase;
class UNiagaraSystem;

UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Combat Feedback"))
class TEAM13_PROJECT_API UCombatFeedbackSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
    // 메뉴 위치 (선택)
    UPROPERTY(EditAnywhere, Config, Category = "UI", meta = (DisplayName = "Category Path"))
    FString DummyCategory = "Game/Combat";

    // 전역 기본 사운드 / 이펙트 (SoftObjectPtr로 안전하게)
    UPROPERTY(EditAnywhere, Config, Category = "Assets")
    TSoftObjectPtr<USoundBase> DefaultHitSFX;

    UPROPERTY(EditAnywhere, Config, Category = "Assets")
    TSoftObjectPtr<UNiagaraSystem> DefaultHitVFX;

    // 임팩트 기반 스케일/볼륨 계수(필요 시)
    UPROPERTY(EditAnywhere, Config, Category = "Tuning")
    float DefaultSFXVolumeBase = 0.9f;

    UPROPERTY(EditAnywhere, Config, Category = "Tuning")
    float DefaultSFXPitchBase = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Blink")
    FName BlinkScalarParam = TEXT("HitBlink");
};

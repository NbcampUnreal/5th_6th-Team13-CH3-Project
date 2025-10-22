// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseMonsterCharacter.generated.h"

UCLASS()
class TEAM13_PROJECT_API ABaseMonsterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	ABaseMonsterCharacter();

	//HP 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float CurrentHealth = 100.f;
	//죽음 확인
	UFUNCTION(BlueprintPure, Category = "Monster\Health")
	bool IsDead() const { return CurrentHealth <= 0; }

	//충돌 이벤트
	UFUNCTION()       //   충돌한 본인 컴포넌트,      충돌 대상,   충돌 대상 컴포넌트,                충격 백터,                충돌지점과 방향등 세부 정보    
	void OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, FVector  NormalImpulse, const FHitResult& Hit);



	//크기에 대한 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float SizeScale = 1.0f;
	//속도에 대한 변수
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float Speed = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	int32 Score = 0;

protected:
	virtual void BeginPlay() override;

	//실재 크기 변환 함수
	void SyncSizeToScale();

};

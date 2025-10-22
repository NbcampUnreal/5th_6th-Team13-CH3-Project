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

	//HP ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float MaxHealth = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float CurrentHealth = 100.f;
	//���� Ȯ��
	UFUNCTION(BlueprintPure, Category = "Monster\Health")
	bool IsDead() const { return CurrentHealth <= 0; }

	//�浹 �̺�Ʈ
	UFUNCTION()       //   �浹�� ���� ������Ʈ,      �浹 ���,   �浹 ��� ������Ʈ,                ��� ����,                �浹������ ����� ���� ����    
	void OnCapsuleHit(UPrimitiveComponent* HitComp, AActor* Other, UPrimitiveComponent* OtherComp, FVector  NormalImpulse, const FHitResult& Hit);



	//ũ�⿡ ���� ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float SizeScale = 1.0f;
	//�ӵ��� ���� ����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	float Speed = 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Monster")
	int32 Score = 0;

protected:
	virtual void BeginPlay() override;

	//���� ũ�� ��ȯ �Լ�
	void SyncSizeToScale();

};

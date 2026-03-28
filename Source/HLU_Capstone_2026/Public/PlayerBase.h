// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "Damageable.h"
#include "PlayerBase.generated.h"

/**
 플레이어 캐릭터의 베이스 클래스입니다. 

 PaperZD 클래스를 상속받으며,
 HealthComponent와의 통신을 위해 IDamageable 인터페이스를 구현합니다. 




 */

UCLASS()
class HLU_CAPSTONE_2026_API APlayerBase : public APaperZDCharacter, public IDamageable
{
	GENERATED_BODY()

// 인터페이스 및 컴포넌트 구현 -------------------
public:
    APlayerBase();

    // IDamageable 인터페이스 구현
    virtual void ReceiveDamage_Implementation(const FDamageData& DamageData) override;

    // IDamageable 인터페이스 구현
    virtual void OnDeath_Implementation() override;

protected:
    // 체력 관리 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UHealthComponent* HealthComponent;

    // 공격 판정 범위 - 박스 형태의 트리거
    UPROPERTY(VisibleAnywhere, Category = "Components")
    class UBoxComponent* AttackBox;


};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "CustomDamageType.generated.h"

/**
 * 공격의 유효성을 정의하는 데미지 타입 클래스입니다.
 * 회피 가능/불가능 공격, 가드 가능/불가능 공격, 무적 무시 공격등에 대해 정의합니다.
 */

UCLASS()
class HLU_CAPSTONE_2026_API UCustomDamageType : public UDamageType
{
	GENERATED_BODY()

public:
    // 회피 가능 여부 (회피 타이밍에 피격될 때 피해를 적용할지)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Properties")
    bool bIgnoreDodge = false;

    // 가드 가능 여부 (가드 타이밍에 피격될 때 피해를 적용할지)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Properties")
    bool bIgnoreGuard = false;

    // 무적상태 무시 여부 (낙뎀과 같은 환경적 요인 피해), 사실상 건드릴 일 없는 플래그이니 신경쓰지 말것
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Damage Properties")
    bool bIgnoreInvincible = false;
	
};

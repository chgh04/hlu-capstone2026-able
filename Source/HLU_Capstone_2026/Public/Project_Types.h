// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Project_Types.generated.h"

// 공통으로 사용되는 구조체 정의를 위한 헤더파일입니다.

// 공격 정보에 대한 구조체 정의
USTRUCT(BlueprintType)
struct FDamageData
{
    GENERATED_BODY()

    // 입힌 피해량
    UPROPERTY(BlueprintReadWrite)
    float DamageAmount = 0.f;

    // 공격을 가한 대상
    UPROPERTY(BlueprintReadWrite)
    AActor* DamageCauser = nullptr;

    // 넉백 처리를 위해 필요
    UPROPERTY(BlueprintReadWrite)
    FVector HitDirection = FVector::ZeroVector;

    // 회피 가능 여부 (회피 타이밍에 피격될 때 피해를 적용할지)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIgnoreDodge = false;

    // 가드 가능 여부 (가드 타이밍에 피격될 때 피해를 적용할지)
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIgnoreGuard = false;

    // 무적상태 무시 여부 (낙뎀과 같은 환경적 요인 피해), 사실상 건드릴 일 없는 플래그이니 신경쓰지 말것
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIgnoreInvincible = false;

};
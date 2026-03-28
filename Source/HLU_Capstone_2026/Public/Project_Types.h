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

    UPROPERTY(BlueprintReadWrite)
    float DamageAmount = 0.f;

    UPROPERTY(BlueprintReadWrite)
    AActor* DamageCauser = nullptr;

    // 넉백 처리를 위해 필요
    UPROPERTY(BlueprintReadWrite)
    FVector HitDirection = FVector::ZeroVector;
};
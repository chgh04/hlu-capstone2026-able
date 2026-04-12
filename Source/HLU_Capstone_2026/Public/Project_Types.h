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

// 아이템 분류
UENUM(BlueprintType)
enum class EItemType : uint8
{
    None        UMETA(DisplayName = "None"),
    Quest       UMETA(DisplayName = "Quest"),       // 퀘스트 아이템
    Upgrade     UMETA(DisplayName = "Upgrade"),     // 강화 아이템
    Relic       UMETA(DisplayName = "Relic"),       // 장신구(성유물)
    Record      UMETA(DisplayName = "Record")       // 일지/기록
};

// 아이템 데이터 구조체
// BaseItem BP의 디테일 패널에서 드롭다운으로 선택 가능
// ItemCode는 세이브/로드 및 인벤토리에서 아이템을 식별하는 고유 키로 사용
USTRUCT(BlueprintType)
struct FPilgrimItemData
{
    GENERATED_BODY()

    // 아이템 고유 코드 - 세이브/로드, 인벤토리 식별용 (예: "ITEM_SWORD_01")
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemCode = NAME_None;

    // 아이템 이름 - UI 표시용
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ItemName = TEXT("Unknown Item");

    // 아이템 설명 - 인벤토리 UI 표시용
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString ItemDescription = TEXT("");

    // 아이템 분류 - 저장 위치 및 처리 방식 구분에 사용
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EItemType ItemType = EItemType::None;

    // 아이템 수량
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ItemAmount = 1;
};
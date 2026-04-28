#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Project_Types.h"
#include "PilgrimSaveGame.generated.h"

/*
UPilgrimSaveGame - 게임 세이브 데이터 클래스

역할
USaveGame을 상속받아 게임 진행 데이터를 저장한다.
체크포인트 통과 / 상호작용 시 자동저장된다.

저장 데이터
- 플레이어 마지막 체크포인트 위치
- 플레이어 현재 체력
- 포션 횟수
- 활성화된 체크포인트 이름 목록
- 보유 아이템 목록
- 장신구 슬롯
*/

UCLASS()
class HLU_CAPSTONE_2026_API UPilgrimSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPilgrimSaveGame();

// 플레이어 상태
public:
    // 마지막으로 활성화된 체크포인트 위치 (부활 위치)
    UPROPERTY(VisibleAnywhere, Category = "Save_Player")
    FVector PlayerRespawnLocation = FVector::ZeroVector;

    // 저장 시점의 플레이어 현재 체력
    UPROPERTY(VisibleAnywhere, Category = "Save_Player")
    float PlayerCurrentHealth = 100.f;

    // 저장 시점의 포션 횟수
    UPROPERTY(VisibleAnywhere, Category = "Save_Player")
    int32 PlayerCurrentPotionCount = 3;

 // 체크포인트 상태
public:
    // 활성화된 체크포인트 이름 목록 - GetName()으로 식별
    UPROPERTY(VisibleAnywhere, Category = "Save_Checkpoint")
    TArray<FString> ActivatedCheckpointNames;

// 인벤토리 상태
public:
    // 보유 아이템 코드 목록 - 유무 판단용
    UPROPERTY(VisibleAnywhere, Category = "Save_Inventory")
    TArray<FName> OwnedItemCodes;

    // 전체 보유 아이템 데이터 목록 - UI 복원용
    UPROPERTY(VisibleAnywhere, Category = "Save_Inventory")
    TArray<FPilgrimItemData> AllItems;

    // 장신구 슬롯 데이터
    UPROPERTY(VisibleAnywhere, Category = "Save_Inventory")
    TArray<FPilgrimItemData> RelicSlots;

    // 활성화된 장신구 슬롯 수
    UPROPERTY(VisibleAnywhere, Category = "Save_Inventory")
    int32 ActiveRelicSlotCount = 1;

// 세이브 슬롯 설정
public:
    // 세이브 슬롯 이름 - UGameplayStatics::SaveGameToSlot에서 사용
    static const FString SaveSlotName;

    // 세이브 유저 인덱스
    static const int32 SaveUserIndex;
};
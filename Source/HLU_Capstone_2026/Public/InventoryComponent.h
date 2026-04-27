#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Project_Types.h"
#include "InventoryComponent.generated.h"

/*
UInventoryComponent - 플레이어 인벤토리 관리 컴포넌트

역할
플레이어가 보유한 아이템 목록과 장신구 슬롯을 관리한다.
아이템 보유 여부는 OwnedItemCodes로 판단하고,
UI 표시용 전체 목록은 AllItems로 관리한다.

사용 방법
PlayerBase 생성자에서 CreateDefaultSubobject로 생성한다.
아이템 습득 시 BaseItem의 OnItemPickedUp 델리게이트에 AddItem을 바인딩한다.

확장 가능한 점
세이브/로드 시 OwnedItemCodes와 RelicSlots만 저장하면 된다.
장신구 슬롯 수는 MaxRelicSlots로 조정 가능하다.
*/

// 아이템 추가 시 UI 갱신용 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnItemAddedSignature, const FPilgrimItemData&, ItemData);

// 장신구 슬롯 변경 시 UI 갱신용 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRelicSlotChangedSignature, int32, SlotIndex);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HLU_CAPSTONE_2026_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

protected:
    virtual void BeginPlay() override;

    // 델리게이트
public:
    // 아이템 추가 시 UI에서 바인딩
    UPROPERTY(BlueprintAssignable, Category = "Inventory_Events")
    FOnItemAddedSignature OnItemAdded;

    // 장신구 슬롯 변경 시 UI에서 바인딩
    UPROPERTY(BlueprintAssignable, Category = "Inventory_Events")
    FOnRelicSlotChangedSignature OnRelicSlotChanged;

    // 아이템 관련 함수/변수
public:
    // 아이템 추가 - BaseItem의 OnItemPickedUp 델리게이트에 바인딩
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void AddItem(const FPilgrimItemData& ItemData);

    // 아이템 보유 여부 확인 - ItemCode 기반
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    bool HasItem(FName ItemCode) const;

    // 전체 보유 아이템 목록 반환 - UI 표시용
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    const TArray<FPilgrimItemData>& GetAllItems() const { return AllItems; }

private:
    // 보유 아이템 코드 목록 - 유무 판단용 (세이브/로드 핵심 데이터)
    UPROPERTY(VisibleAnywhere, Category = "Inventory")
    TArray<FName> OwnedItemCodes;

    // 전체 보유 아이템 데이터 목록 - UI 표시용
    UPROPERTY(VisibleAnywhere, Category = "Inventory")
    TArray<FPilgrimItemData> AllItems;

    // 장신구 슬롯 관련 함수/변수
public:
    // 장신구 슬롯에 장착 - SlotIndex: 0부터 시작
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    bool EquipRelic(const FPilgrimItemData& ItemData, int32 SlotIndex);

    // 장신구 슬롯 해제
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UnequipRelic(int32 SlotIndex);

    // 특정 슬롯의 장신구 반환
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    FPilgrimItemData GetRelicAtSlot(int32 SlotIndex) const;

    // 현재 활성화된 장신구 슬롯 수 반환
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Inventory")
    int32 GetActiveRelicSlotCount() const { return ActiveRelicSlotCount; }

    // 장신구 슬롯 수 증가 - 게임 진행에 따라 슬롯 해금 시 호출
    UFUNCTION(BlueprintCallable, Category = "Inventory")
    void UnlockRelicSlot();

private:
    // 장신구 슬롯 배열 - 최대 MaxRelicSlots개, 빈 슬롯은 ItemCode == NAME_None
    UPROPERTY(VisibleAnywhere, Category = "Inventory")
    TArray<FPilgrimItemData> RelicSlots;

    // 장신구 최대 슬롯 수
    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    int32 MaxRelicSlots = 4;

    // 현재 활성화된 슬롯 수 - 처음엔 1개, 게임 진행에 따라 증가
    UPROPERTY(EditDefaultsOnly, Category = "Inventory")
    int32 ActiveRelicSlotCount = 1;
};
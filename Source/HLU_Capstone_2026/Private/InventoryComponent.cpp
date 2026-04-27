#include "InventoryComponent.h"

UInventoryComponent::UInventoryComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    // 장신구 슬롯 배열 MaxRelicSlots 크기로 초기화
    // 빈 슬롯은 ItemCode == NAME_None으로 구분
    RelicSlots.SetNum(MaxRelicSlots);
}

void UInventoryComponent::AddItem(const FPilgrimItemData& ItemData)
{
    // 이미 보유한 아이템이면 수량만 증가
    if (HasItem(ItemData.ItemCode))
    {
        for (FPilgrimItemData& Existing : AllItems)
        {
            if (Existing.ItemCode == ItemData.ItemCode)
            {
                Existing.ItemAmount += ItemData.ItemAmount;
                break;
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("Inventory: [%s] amount increased"), *ItemData.ItemName);
        OnItemAdded.Broadcast(ItemData);
        return;
    }

    // 새 아이템 추가
    OwnedItemCodes.Add(ItemData.ItemCode);
    AllItems.Add(ItemData);

    UE_LOG(LogTemp, Warning, TEXT("Inventory: [%s] added. Total items: %d"), *ItemData.ItemName, AllItems.Num());

    OnItemAdded.Broadcast(ItemData);
}

bool UInventoryComponent::HasItem(FName ItemCode) const
{
    return OwnedItemCodes.Contains(ItemCode);
}

bool UInventoryComponent::EquipRelic(const FPilgrimItemData& ItemData, int32 SlotIndex)
{
    // 유효한 슬롯 범위인지 확인
    if (SlotIndex < 0 || SlotIndex >= ActiveRelicSlotCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("Inventory: Invalid relic slot index %d"), SlotIndex);
        return false;
    }

    // 장신구 타입인지 확인
    if (ItemData.ItemType != EItemType::Relic)
    {
        UE_LOG(LogTemp, Warning, TEXT("Inventory: [%s] is not a Relic type"), *ItemData.ItemName);
        return false;
    }

    // 슬롯에 장착
    RelicSlots[SlotIndex] = ItemData;

    UE_LOG(LogTemp, Warning, TEXT("Inventory: [%s] equipped to slot %d"), *ItemData.ItemName, SlotIndex);

    OnRelicSlotChanged.Broadcast(SlotIndex);
    return true;
}

void UInventoryComponent::UnequipRelic(int32 SlotIndex)
{
    // 유효한 슬롯 범위인지 확인
    if (SlotIndex < 0 || SlotIndex >= ActiveRelicSlotCount)
    {
        UE_LOG(LogTemp, Warning, TEXT("Inventory: Invalid relic slot index %d"), SlotIndex);
        return;
    }

    // 슬롯 비우기
    RelicSlots[SlotIndex] = FPilgrimItemData();

    UE_LOG(LogTemp, Warning, TEXT("Inventory: Slot %d unequipped"), SlotIndex);

    OnRelicSlotChanged.Broadcast(SlotIndex);
}

FPilgrimItemData UInventoryComponent::GetRelicAtSlot(int32 SlotIndex) const
{
    if (SlotIndex < 0 || SlotIndex >= RelicSlots.Num())
    {
        return FPilgrimItemData();
    }

    return RelicSlots[SlotIndex];
}

void UInventoryComponent::UnlockRelicSlot()
{
    if (ActiveRelicSlotCount >= MaxRelicSlots)
    {
        UE_LOG(LogTemp, Warning, TEXT("Inventory: Already at max relic slots (%d)"), MaxRelicSlots);
        return;
    }

    ActiveRelicSlotCount++;

    UE_LOG(LogTemp, Warning, TEXT("Inventory: Relic slot unlocked! Active slots: %d"), ActiveRelicSlotCount);
}

void UInventoryComponent::LoadFromSaveData(
    const TArray<FName>& InOwnedItemCodes,
    const TArray<FPilgrimItemData>& InAllItems,
    const TArray<FPilgrimItemData>& InRelicSlots,
    int32 InActiveRelicSlotCount)
{
    OwnedItemCodes = InOwnedItemCodes;
    AllItems = InAllItems;
    RelicSlots = InRelicSlots;
    ActiveRelicSlotCount = InActiveRelicSlotCount;

    // 로드된 아이템 각각에 대해 델리게이트 브로드캐스트 - UI 갱신용
    for (const FPilgrimItemData& Item : AllItems)
    {
        OnItemAdded.Broadcast(Item);
    }

    UE_LOG(LogTemp, Warning, TEXT("Inventory: Loaded %d items from save"), AllItems.Num());
}
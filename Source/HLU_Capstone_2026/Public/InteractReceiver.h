#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "InteractReceiver.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UInteractReceiver : public UInterface
{
    GENERATED_BODY()
};

class HLU_CAPSTONE_2026_API IInteractReceiver
{
    GENERATED_BODY()

public:
// 아이템 등록/해제
    // 아이템이 오버랩 범위에 들어올 때 플레이어에게 자신을 등록
    UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
    void RegisterNearbyItem(AActor* Item);

    // 아이템이 오버랩 범위를 벗어날 때 플레이어에서 자신을 해제
    UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
    void UnregisterNearbyItem(AActor* Item);

// 인터랙터블 등록/해제
    // 인터랙터블이 오버랩 범위에 들어올 때 플레이어에게 자신을 등록
    UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
    void RegisterNearbyInteractable(AActor* Interactable);

    // 인터랙터블이 오버랩 범위를 벗어날 때 플레이어에서 자신을 해제
    UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
    void UnregisterNearbyInteractable(AActor* Interactable);

    // 플레이어에서 InteractableBase에게 TryInteract 실행 신호 전달
    UFUNCTION(BlueprintNativeEvent, Category = "Interaction")
    void TryInteract(AActor* Interactable);
};


/*
IInteractReceiver - 아이템 / 인터랙터블 근접 등록 인터페이스

역할
ABaseItem, AInteractableBase가 플레이어와 오버랩될 때 직접 이 인터페이스를 통해
플레이어에게 자신을 등록하고 해제, 이후 APlayerBase가 구현

사용 흐름
1. 아이템/인터랙터블이 플레이어와 Overlap Begin → Execute_Register...() 호출
2. 아이템/인터랙터블이 플레이어와 Overlap End   → Execute_Unregister...() 호출
3. BP_Player에서 F키 Started → HandleInteractInput() 호출
   → 등록된 NearbyItems / NearbyInteractables 순회하여 처리
*/
#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Rootable.generated.h"

/*
IRootable - 습득 가능한 아이템 인터페이스

역할
플레이어가 접촉했을 때 습득할 수 있는 모든 아이템이 구현하는 인터페이스.
ABaseItem을 상속받는 모든 클래스에서 구현합니다.

사용 방법
ABaseItem을 상속받는 클래스에서 이미 구현되어 있습니다.
별도의 아이템 클래스를 만들 경우 IRootable을 다중 상속받고
OnPickedUp_Implementation을 구현하세요.

확장 가능한 점
아이템 등급 시스템 추가 시 EItemRarity 열거형을 Project_Types.h에 추가
장비 아이템의 경우 OnEquipped / OnUnEquipped 함수를 추가 가능
*/

UINTERFACE(MinimalAPI, Blueprintable)
class URootable : public UInterface
{
    GENERATED_BODY()
};

class HLU_CAPSTONE_2026_API IRootable
{
    GENERATED_BODY()

public:
    // 아이템을 습득했을 때 호출 - 습득한 액터(플레이어)를 전달
    UFUNCTION(BlueprintNativeEvent, Category = "Item")
    void OnPickedUp(AActor* Picker);

    // 아이템을 사용했을 때 호출
    UFUNCTION(BlueprintNativeEvent, Category = "Item")
    void OnUsed(AActor* User);
};

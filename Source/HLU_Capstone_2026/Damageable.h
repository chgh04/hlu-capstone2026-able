#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UDamageable : public UInterface
{
    GENERATED_BODY()
};

// 실제 인터페이스 클래스 - 이걸 상속받아서 사용
class HLU_CAPSTONE_2026_API IDamageable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, Category = "Damage")
    void ReceiveDamage(float Amount); //데미지 수신 함수 - 액터의 TakeDamage가 호출될 때 내부에서 이 함수를 호출 

    UFUNCTION(BlueprintNativeEvent, Category = "Damage")
    void OnDeath();//사망 처리 함수 - HealthComponent에서 체력이 0이 됐을 때 오너 액터에게 직접 호출
};
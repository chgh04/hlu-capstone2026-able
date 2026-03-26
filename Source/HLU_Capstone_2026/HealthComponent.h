#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

// 체력 변경 시 호출되는 델리게이트
// 현재 체력, 최대 체력을 넘겨줌 나중에 UI에서 바인딩
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChanged, float, CurrentHealth, float, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDie);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HLU_CAPSTONE_2026_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();

    // 외부(Enemy, Player 등)에서 호출 - 데미지 처리
    UFUNCTION(BlueprintCallable, Category = "Health")
    void ReduceHealth(float Amount);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
    float GetCurrentHealth() const { return CurrentHealth; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
    float GetMaxHealth() const { return MaxHealth; }

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
    bool IsDead() const { return bIsDead; }

    // UI, 애니메이션 등에서 바인딩
    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnHealthChanged OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Health")
    FOnDie OnDie;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(EditAnywhere, Category = "Health", meta = (ClampMin = "1.0"))
    float MaxHealth = 100.f;

    UPROPERTY(VisibleAnywhere, Category = "Health")
    float CurrentHealth = 0.f;

    UPROPERTY(VisibleAnywhere, Category = "Health")
    bool bIsDead = false;
};
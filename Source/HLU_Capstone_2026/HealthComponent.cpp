#include "HealthComponent.h"

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false; // 매 프레임 필요 없음
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth; // 시작 시 최대 체력으로 초기화
}

void UHealthComponent::ReduceHealth(float Amount)
{
    // 이미 죽었으면 무시
    if (bIsDead || Amount <= 0.f) return;

    CurrentHealth = FMath::Clamp(CurrentHealth - Amount, 0.f, MaxHealth);

    // UI 등에 알림
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

    if (CurrentHealth <= 0.f)
    {
        bIsDead = true;
        OnDie.Broadcast();
    }
}
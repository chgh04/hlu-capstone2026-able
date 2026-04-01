#include "HealthComponent.h"
#include "Damageable.h" 

UHealthComponent::UHealthComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UHealthComponent::BeginPlay()
{
    Super::BeginPlay();

    // 게임 시작 시 현재 체력을 최대 체력으로 초기화
    CurrentHealth = MaxHealth;

    // UI에게 체력 알림
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);
}

void UHealthComponent::ReduceHealth(const FDamageData& DamageData)
{
    // 이미 사망했거나 데미지가 0 이하인 경우
    if (bIsDead || DamageData.DamageAmount <= 0.f) return;

    // 체력 감소 - 0 미만으로 내려가지 않도록 Clamp 처리
    CurrentHealth = FMath::Clamp(CurrentHealth - DamageData.DamageAmount, 0.f, MaxHealth);

    // 델리게이트를 통해 체력 변경 알림 (UI 갱신용)
    OnHealthChanged.Broadcast(CurrentHealth, MaxHealth);

    // 델리게이트를 통해 피해정보 전달
    OnTakeDamage.Broadcast(DamageData);

    UE_LOG(LogTemp, Warning, TEXT("C++: Got Damaged %2.f, Remain HP: %.2f"), DamageData.DamageAmount, CurrentHealth);

    // 체력이 0이 되면 사망 처리
    if (CurrentHealth <= 0.f)
    {
        // 중복 사망 방지 플래그
        bIsDead = true;

        // 오너 액터가 IDamageable을 구현하고 있는지 확인 후 OnDeath 호출
        // 인터페이스로 직접 호출
        AActor* Owner = GetOwner();
        if (Owner && Owner->Implements<UDamageable>())
        {
            IDamageable::Execute_OnDeath(Owner);
        }
    }
}

void UHealthComponent::HealHealth(float Amount)
{
    // 이미 사망했거나 데미지가 0 이하인 경우
    if (bIsDead || Amount <= 0.f) return;

    // 체력 회복, 최대최력 이상으로 회복되지 않음
    CurrentHealth = FMath::Clamp(CurrentHealth + Amount, 0.f, MaxHealth);
}

void UHealthComponent::SetMaxHealth(float Amount)
{
    MaxHealth = Amount;
}
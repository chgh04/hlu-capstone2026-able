#include "CheckpointBase.h"
#include "PlayerBase.h"
#include "HealthComponent.h"

ACheckpointBase::ACheckpointBase()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ACheckpointBase::BeginPlay()
{
    Super::BeginPlay();
}

void ACheckpointBase::OnInteract_Implementation(AActor* Interactor)
{
    Super::OnInteract_Implementation(Interactor);

    // 처음 상호작용이면 체크포인트 활성화
    if (!bIsActivated)
    {
        ActivateCheckpoint(Interactor);
    }

    // 매 상호작용마다 체력 회복 + 포션 재공급
    HealPlayer(Interactor);
    RefillPotion(Interactor);
}

void ACheckpointBase::ActivateCheckpoint(AActor* Interactor)
{
    bIsActivated = true;

    // 활성화 이펙트 재생 (자식 BP에서 구현)
    PlayActivateEffect();

    // 나이아가라 이펙트 켜기
    if (InteractEffect)
    {
        InteractEffect->Activate(true);
    }

    UE_LOG(LogTemp, Warning, TEXT("Checkpoint: Activated!"));
}

void ACheckpointBase::HealPlayer(AActor* Interactor)
{
    APlayerBase* Player = Cast<APlayerBase>(Interactor);
    if (!Player) return;

    // HealthComponent 가져오기
    UHealthComponent* HealthComp = Player->FindComponentByClass<UHealthComponent>();
    if (!HealthComp) return;

    // HealPercent만큼 회복
    // 예: HealPercent = 1.0이면 최대 체력만큼 회복
    float HealAmount = HealthComp->GetMaxHealth() * HealPercent;
    HealthComp->HealHealth(HealAmount);

    PlayHealEffect();

    UE_LOG(LogTemp, Warning, TEXT("Checkpoint: Player healed by %.1f%%"), HealPercent * 100.f);
}

void ACheckpointBase::RefillPotion(AActor* Interactor)
{
    // 상호작용한 액터가 플레이어인지 캐스팅하여 확인
    APlayerBase* Player = Cast<APlayerBase>(Interactor);
    if (!Player) return;

    // 플레이어의 포션 충전 함수 호출
    Player->RefillPotion();
}
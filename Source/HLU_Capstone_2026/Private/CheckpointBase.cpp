#include "CheckpointBase.h"
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
    // Super::OnInteract_Implementation(Interactor);

    UE_LOG(LogTemp, Warning, TEXT("Checkpoint: OnInteract Called, %s"), *GetName());

    // 처음 상호작용이면 체크포인트 활성화
    if (!bIsActivated)
    {
        ActivateCheckpoint(Interactor);
    }

    if (Interactor->Implements<UCheckpointInteractable>())
    {
        // 인터페이스 함수 호출
        ICheckpointInteractable::Execute_RestAtCheckpoint(Interactor, HealPercent, this);

        // 휴식 중 플래그 전환
        bIsResting = true;

        // 포스트 프로세스 적용
        OnRestStartedVisuals();
    }
}

void ACheckpointBase::EndCheckpointRest_Implementation()
{   
    // 이 체크포인트 오브젝트와 상호작용중에만 실행
    if (bIsResting)
    {   
        // 포스트 프로세스 초기화
        OnRestEndedVisuals();
    }
}

void ACheckpointBase::ActivateCheckpoint(AActor* Interactor)
{
    bIsActivated = true;

    // 활성화 이펙트 재생 (자식 BP에서 구현)
    PlayActivateEffect();

    // 나이아가라 이펙트 켜기
    /*if (InteractEffect)
    {
        InteractEffect->Activate(true);
    }*/

    UE_LOG(LogTemp, Warning, TEXT("Checkpoint: Activated!"));
}
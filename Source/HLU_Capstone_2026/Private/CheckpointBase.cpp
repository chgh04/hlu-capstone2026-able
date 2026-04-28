#include "CheckpointBase.h"
#include "HealthComponent.h"
#include "SaveLoadComponent.h"

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

    if (Interactor->Implements<UCheckpointInteractable>() && !bIsResting)
    {
        // 인터페이스 함수 호출
        ICheckpointInteractable::Execute_RestAtCheckpoint(Interactor, HealPercent, this);   // 플레이어 회복
        ICheckpointInteractable::Execute_SaveAtCheckpoint(Interactor, GetActorLocation());  // 플레이어 상태 저장

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

        // 휴식 플래그 초기화
        bIsResting = false;
    }
}

void ACheckpointBase::ActivateCheckpoint(AActor* Interactor)
{
    bIsActivated = true;

    PlayActivateEffect();

    // 플레이어의 SaveLoadComponent에 이름 등록
    if (Interactor)
    {
        USaveLoadComponent* SaveComp = Interactor->FindComponentByClass<USaveLoadComponent>();
        if (SaveComp)
        {
            SaveComp->RegisterActivatedCheckpoint(CheckpointName);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("Checkpoint: Activated! Name: %s"), *CheckpointName);
}

//댕글링 포인터 크래쉬 방지
void ACheckpointBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}
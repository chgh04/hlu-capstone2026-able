#pragma once

#include "CoreMinimal.h"
#include "InteractableBase.h"
#include "CheckpointInteractable.h"
#include "CheckpointBase.generated.h"

UCLASS()
class HLU_CAPSTONE_2026_API ACheckpointBase : public AInteractableBase, public ICheckpointInteractable
{
    GENERATED_BODY()

public:
    ACheckpointBase();

protected:
    virtual void BeginPlay() override;

// 컴포넌트 생성
protected:

// 휴식 기능 관현 함수/변수
public:
    // 체력 회복량 (0.0 ~ 1.0, 1.0 = 전체 회복)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
    float HealPercent = 1.0f;

    // 이미 활성화된 체크포인트인지
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
    bool bIsActivated = false;

    // 체크포인트 인터페이스 함수 전달 (체력 및 회복약 개수 충전)
    virtual void OnInteract_Implementation(AActor* Interactor) override;

    // 체크포인트 활성화 - 처음 상호작용 시 1회 호출
    UFUNCTION(BlueprintCallable, Category = "Checkpoint")
    void ActivateCheckpoint(AActor* Interactor);

// 이펙트 및 사운드
protected:
    // 자식 BP에서 구현 - 활성화 애니메이션/이펙트
    UFUNCTION(BlueprintImplementableEvent, Category = "Checkpoint")
    void PlayActivateEffect();

    // 자식 BP에서 구현 - 체력 회복 이펙트
    UFUNCTION(BlueprintImplementableEvent, Category = "Checkpoint")
    void PlayHealEffect();
};
#pragma once

#include "CoreMinimal.h"
#include "InteractableBase.h"
#include "CheckpointBase.generated.h"

UCLASS()
class HLU_CAPSTONE_2026_API ACheckpointBase : public AInteractableBase
{
    GENERATED_BODY()

public:
    ACheckpointBase();

protected:
    virtual void BeginPlay() override;

public:
    // 체력 회복량 (0.0 ~ 1.0, 1.0 = 전체 회복)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
    float HealPercent = 1.0f;

    // 포션 재공급 횟수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
    int32 PotionRefillCount = 3;

    // 이미 활성화된 체크포인트인지
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
    bool bIsActivated = false;

public:
    virtual void OnInteract_Implementation(AActor* Interactor) override;

    // 체크포인트 활성화 - 처음 상호작용 시 1회 호출
    UFUNCTION(BlueprintCallable, Category = "Checkpoint")
    void ActivateCheckpoint(AActor* Interactor);

    // 체력 회복 실행
    UFUNCTION(BlueprintCallable, Category = "Checkpoint")
    void HealPlayer(AActor* Interactor);

    // 포션 재공급 실행
    UFUNCTION(BlueprintCallable, Category = "Checkpoint")
    void RefillPotion(AActor* Interactor);

protected:
    // 자식 BP에서 구현 - 활성화 애니메이션/이펙트
    UFUNCTION(BlueprintImplementableEvent, Category = "Checkpoint")
    void PlayActivateEffect();

    // 자식 BP에서 구현 - 체력 회복 이펙트
    UFUNCTION(BlueprintImplementableEvent, Category = "Checkpoint")
    void PlayHealEffect();
};
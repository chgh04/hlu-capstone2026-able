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

// 휴식 기능 관련 함수/변수
public:
    // 체력 회복량 (0.0 ~ 1.0, 1.0 = 전체 회복)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
    float HealPercent = 1.0f;

    // 이미 활성화된 체크포인트인지
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Checkpoint")
    bool bIsActivated = false;

    // 체크포인트 인터페이스 함수 전달 (체력 및 회복약 개수 충전)
    virtual void OnInteract_Implementation(AActor* Interactor) override;

    // 체크포인트 종료 시그널 수신 (PostProcess 및 각종 환경이펙트 초기화)
    virtual void EndCheckpointRest_Implementation() override;

    // 체크포인트 활성화 - 처음 상호작용 시 1회 호출
    UFUNCTION(BlueprintCallable, Category = "Checkpoint")
    void ActivateCheckpoint(AActor* Interactor);

    // 플레이어가 해당 체크포인트에서 휴식중인지
    UPROPERTY(VisibleAnywhere, Category = "Checkpoint")
    bool bIsResting = false;
    // 체크포인트 이름을 세이브 데이터에 기록 - 활성화 목록 복원용
    UFUNCTION(BlueprintCallable, Category = "Checkpoint")
    const FString& GetCheckpointName() const { return CheckpointName; }

protected:
    // 이 체크포인트의 고유 이름 - 디테일 패널에서 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checkpoint")
    FString CheckpointName = TEXT("Checkpoint_Default");

// 이펙트 및 사운드
protected:
    // 자식 BP에서 구현 - 활성화 애니메이션/이펙트
    UFUNCTION(BlueprintImplementableEvent, Category = "Checkpoint")
    void PlayActivateEffect();

    // 자식 BP에서 구현 - 체력 회복 이펙트(현재 미사용)
    UFUNCTION(BlueprintImplementableEvent, Category = "Checkpoint")
    void PlayHealEffect();

    // 휴식 시작 시 블루프린트에서 DoF 타임라인 Play를 실행할 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Checkpoint")
    void OnRestStartedVisuals();

    // 휴식 종료 시 블루프린트에서 DoF 타임라인 Reverse를 실행할 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Checkpoint")
    void OnRestEndedVisuals();
//댕글링 포인터 크래쉬 방지
protected:
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
};
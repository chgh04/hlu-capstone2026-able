#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "MapTransitionTrigger.generated.h"

// 암전/복구 연출을 BP에 알리기 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTransitionFadeOutSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTransitionFadeInSignature);

UCLASS()
class HLU_CAPSTONE_2026_API AMapTransitionTrigger : public AActor
{
    GENERATED_BODY()

public:
    AMapTransitionTrigger();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    // 컴포넌트
protected:
    // 플레이어 오버랩 감지용
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBoxComponent* TriggerBox;

    // 전환 설정
public:
    // 로드할 서브레벨 이름 - 디테일 패널에서 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    FName TargetLevelName;

    // 언로드할 서브레벨 이름 - 디테일 패널에서 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    FName CurrentLevelName;

    // 이동할 입구 태그 - MapEntrance의 EntranceTag와 매칭
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    FGameplayTag TargetEntranceTag;

    // 자동 이동 방향 (1 = 오른쪽, -1 = 왼쪽)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float MoveDirection = 1.f;

    // 암전 유지 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float BlackoutDuration = 1.f;

    // 자동 이동 시간 - 암전 전까지 캐릭터가 걷는 시간
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transition")
    float AutoMoveDuration = 0.5f;

    // 델리게이트
public:
    // 암전 시작 - BP에서 UMG 페이드 아웃 연결
    UPROPERTY(BlueprintAssignable, Category = "Transition")
    FOnTransitionFadeOutSignature OnFadeOut;

    // 암전 해제 - BP에서 UMG 페이드 인 연결
    UPROPERTY(BlueprintAssignable, Category = "Transition")
    FOnTransitionFadeInSignature OnFadeIn;

    // 내부 로직
private:
    UFUNCTION()
    void OnTriggerBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    // 자동 이동 종료 후 암전 + 텔레포트 실행
    void StartFadeAndTransition();

    // 암전 후 레벨 스트리밍 + 텔레포트 실행
    void ExecuteTransition();

    // 텔레포트 후 암전 해제
    void FinishTransition();

    // 전환 중 중복 진입 방지
    bool bIsTransitioning = false;

    // 자동 이동 타이머
    FTimerHandle AutoMoveTimerHandle;

    // 암전 타이머
    FTimerHandle BlackoutTimerHandle;

    // 전환 중인 플레이어 레퍼런스
    UPROPERTY()
    ACharacter* TransitioningPlayer = nullptr;

    // 레벨 로드 완료 대기 타이머
    FTimerHandle WaitForLevelTimerHandle;

    // 레벨 로드 완료 후 입구로 텔레포트
    void TryTeleportToEntrance();
};
#pragma once

#include "CoreMinimal.h"
#include "DefaultCharBase.h"
#include "EnemyBase.generated.h"

// 전방 선언 - include 대신 사용해서 컴파일 속도 향상
class UHealthComponent;
class UBoxComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
    Patrol      UMETA(DisplayName = "순찰"),
    Chase       UMETA(DisplayName = "추격"),
    Attack      UMETA(DisplayName = "공격"),
    Hit         UMETA(DisplayName = "피격"),
    Dead        UMETA(DisplayName = "사망")
};

UCLASS()
class HLU_CAPSTONE_2026_API AEnemyBase : public ADefaultCharBase
{
    GENERATED_BODY()

public:
    AEnemyBase();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

// 인터페이스 구현 및 상속 -------------------
public:
    // 부모 클래스에서 override
    virtual void OnDeath_Implementation() override;

// 컴포넌트 생성 -------------------
protected:
    // 플레이어 감지 범위 - 구 형태의 트리거
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USphereComponent* DetectionRange;

// Enemy 공격 기능 함수/변수 -------------------
protected:
    // 상속받은 Attack_Implemetation 함수 구현
    virtual void Attack_Implementation() override;

    // 공격 대상을 구분하는 함수(플레이어) 구체화
    virtual bool CanAttackTarget(AActor* Target) const;

    virtual bool GetHit(const FDamageData& DamageData) override;

    // 공격 사거리, TODO: Attack Overlap Box와 같은 사이즈로 전환
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    float AttackRange = 100.0f;

// 플레이어 감지(AI) 관련 함수/변수 -------------------
protected:
    // 코드구현 간단 FSM 사용 여부, 디테일 패널에서 끄고 켤 수 있음
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy_AI")
    bool bUseSimpleFSM = true;

    // 감지된 플레이어 저장 - 블루프린트에서 이동 로직에 활용, nullptr: 감지 안된 상태
    UPROPERTY(BlueprintReadOnly, Category = "Enemy_AI")
    class APlayerBase* TargetPlayer;

    // Dectection Range 오버랩 시작 이벤트
    UFUNCTION()
    void OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    // Dectection Range 오버랩 종료 이벤트
    UFUNCTION()
    void OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    // 플레이어 감지 반경 - 블루프린트 디테일 패널에서 적마다 조정 가능
    UPROPERTY(EditAnywhere, Category = "Enemy_AI")
    float DetectionRadius = 500.f;

// 상태머신(SFSM) 관련 함수/변수 -------------------
protected:
    // 상태머신 - SimpleFSM 사용시 적의 이동 코드
    void ChaseOnSimpleFSM();

    // 상태머신 - 공격 종료 및 공격중단 신호 반환
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CallAttackEndOnSimpleFSM();

    // 상태머신 - 경직 종료 후 실행될 함수, 상태머신 최적화용
    void ResetHitStateOnSimpleFSM();

private:
    // AI 상태
    EEnemyState CurrentState;

// 기타 추가 기능
protected:
    FTimerHandle HitStunTimerHandle;
};
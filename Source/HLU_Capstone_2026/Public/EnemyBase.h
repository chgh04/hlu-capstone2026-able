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
    virtual void TryAttack() override;

    // 상속받은 Attack_Implemetation 함수 구현
    virtual void Attack_Implementation() override;

    // 공격 대상을 구분하는 함수(플레이어) 구체화
    virtual bool CanAttackTarget(AActor* Target) const;

    // 공격 사거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy_Combat")
    float AttackRange = 100.0f;

    // 애니메이션 노티파이에서 호출할 전진 스텝 함수 
    UFUNCTION(BlueprintCallable, Category = "Enemy_Combat")
    void StepForward();

    // 노티파이에서 공중 몬스터가 호출할 전진 스텝 함수
    UFUNCTION(BlueprintCallable, Category = "Enemy_Combat")
    void StepFlyingForward(AActor* TargetActor);

    // 캐릭터의 공격 시 전진성, 만약 패턴마다 다르게 하고싶다면 노티파이 등에서 변경 가능합니다. 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy_Combat")
    float EnemyAttackStepForce = 200.f;

    // 공중 몬스터가 캐릭터 공격시 전진성, 노티파이에서 변경 가능합니다.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy_Combat")
    float EnemyFlyingAttackStepForce = 200.f;

    // 공격 쿨타임 관련 변수
    UPROPERTY(EditDefaultsOnly, Category = "Enemy_Combat")
    float MinAttackCooldown = 1.0f;

    // 공격 쿨타임 관련 변수
    UPROPERTY(EditDefaultsOnly, Category = "Enemy_Combat")
    float MaxAttackCooldown = 2.0f;

    // 공격 쿨타임이 돌아 공격 가능한 상태인지 구분 플래그
    bool bCanAttack = true;

    // 쿨타임 초기화 함수
    void ResetAttackCooldown();
    
public:
    // 캐릭터의 공격 시 전진성 변경 함수
    UFUNCTION(BlueprintCallable, Category = "Enemy_Combat")
    void SetEnemyAttackStepForce(float Value) { EnemyAttackStepForce = Value; }

// Enemy 피격 기능 함수/변수 -------------------
protected:
    virtual bool GetHit(const FDamageData& DamageData) override;

    // 캐릭터가 피격중인 상태일 경우
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy_Combat")
    bool bIsHit = false;

    // 디졸브를 사용하지 않을 경우, 사망 후 삭제 대기시간
    UPROPERTY(EditDefaultsOnly, Category = "Enemy_Status")
    float DestroyDelayAfterDeath = 1.0f;

    // 액터 삭제 함수 
    void DestroyEnemy();

// Enemy 이동/회피 관련 함수/변수 -------------------
protected:
    // 비행유닛 공기저항
    float FlyingEnemyFallingLateralFriction = 0.0f;

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

// 공중 유닛 전용 변수
 protected:
     // 공중 유닛 여부 판별 플래그 
     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy_AI")
     bool bIsFlyingEnemy = false;

     // 공중 유닛에게 적용할 중력값
     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy_FlyingOption")
     float FlyingGravity = 0.1f;

     // 타겟 추격시 높이 조정용 변수 
     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy_FlyingOption")
     float FlyTargetHeight = 150.0f;

     //  추격 시 좌우(X축) 무빙 반경 
     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy_FlyingOption")
     float FlySinRangeX = 150.0f;

     //  추격 시 상하(Z축) 출렁임 반경
     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy_FlyingOption")
     float FlySinRangeZ = 50.0f;

     //  공격 시 타겟의 높이 오프셋 
     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy_Combat_Flying")
     float FlyTargetZOffset = 50.0f;
     
     //  공중 유닛이 공격을 시작하기 위해 허용되는 높이 오차범위
     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy_Combat_Flying")
     float FlyingAttackTolerance = 0.0f;

     // 노티파이에서 호출할 위로 밀어올리는 힘
     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy_Combat_Flying")
     float FlyingStrength = 100.0f;

     // 노티파이에서 호출할 위로 떨어지는 힘
     UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy_Combat_Flying")
     float FallingStrength = 50.0f;

     // 노티파이에서 호출할 위로 올리는 함수
     UFUNCTION(BlueprintCallable, Category = "Enemy_Combat_Flying")
     void IdleFlyingUp();

     // 노티파이에서 호출할 밑으로 내리는 함수
     UFUNCTION(BlueprintCallable, Category = "Enemy_Combat_Flying")
     void IdleFlyingDown();

// 상태머신(SFSM) 관련 함수/변수 -------------------
protected:
    // 상태머신 - SimpleFSM 사용시 적의 이동 코드
    void ChaseOnSimpleFSM();

    // 상태머신 - 공격 종료 및 공격중단 신호 반환
    UFUNCTION(BlueprintCallable, Category = "Enemy_Combat")
    void CallAttackEndOnSimpleFSM();

    // 상태머신 - 경직 종료 후 실행될 함수, 상태머신 최적화용
    void ResetHitStateOnSimpleFSM();

private:
    // AI 상태
    EEnemyState CurrentState;

// 기타 추가 기능
protected:
    // 적 캐릭터가 행동 가능한 상태인지 반환
    virtual bool IsCharacterCanAction() override;

    // 적 캐릭터의 피격후 효과 제외 모든 타이머 및 변수 초기화
    virtual void ResetCombatStates() override;

    // 피격시 Hit 상태 타이머 관리자 
    FTimerHandle HitStunTimerHandle;

    // 공격 쿨타임 타이머 관리자
    FTimerHandle AttackCooldownTimerHandle;

    // 피격시 메테리얼 초기화 타이머 관리자
    FTimerHandle HitFlashResetTimerHandle;

    // 사망시 디졸브 텍스처 적용 딜레이 타이머 관리자 
    FTimerHandle DissolveDelayTimerHandle;

    // 디졸브 텍스처를 쓰지 않는 경우에 사용할 삭제 대기 타이머 관리자
    FTimerHandle DeathDestroyTimerHandle;
   
// VFX/오디오 
protected:
    // 피격 후 번쩍이는 효과 초기화
    void ResetHitFlash();

    // 블루프린트에서 M_EnemyHit 머티리얼을 할당할 변수
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy_VFX")
    class UMaterialInterface* HitMaterial;

    // 원래 가지고 있던 머티리얼을 기억해둘 변수
    UPROPERTY()
    class UMaterialInterface* OriginalMaterial;

    // 캐릭터 사망시 디졸브 메테리얼 발생시킬 플래그
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy_VFX")
    bool bIsDissolveWhenDied = true;

    // 동적으로 파라미터를 변경할 동적 머티리얼
    UPROPERTY()
    class UMaterialInstanceDynamic* DissolveMaterial;

    // 사망 후 디졸브가 시작되기까지의 대기 시간 (예: 1초)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy_VFX")
    float DissolveDelay = 2.0f;

    // 캐릭터 디졸브 효과 진행 속도, 1이면 1초만에 사라지고 2면 0.5초만에 사라짐
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enemy_VFX")
    float DissolveSpeed = 1.0f;

    // 현재 디졸브 수치 (0.0 ~ 1.0)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy_VFX")
    float CurrentDissolveAmount = 0.0f;

    // 디졸브 진행 상태 플래그
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy_VFX")
    bool bIsDissolving = false;

    // 사망 대시기간 타이머 이후 호출될 함수
    void StartDissolve();
};
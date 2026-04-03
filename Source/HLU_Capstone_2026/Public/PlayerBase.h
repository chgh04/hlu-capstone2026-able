#pragma once

#include "CoreMinimal.h"
#include "DefaultCharBase.h"
#include "PlayerBase.generated.h"

/**
 플레이어 캐릭터의 베이스 클래스입니다. 

 ADefaultCharBase 클래스를 상속받으며,
 HealthComponent와의 통신을 위해 IDamageable 인터페이스를 구현합니다. 
 */

UCLASS()
class HLU_CAPSTONE_2026_API APlayerBase : public ADefaultCharBase
{
	GENERATED_BODY()

public:
    APlayerBase();

protected:
    virtual void BeginPlay() override;

// 컴포넌트 생성 -------------------
protected:
    // 카메라 암
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class USpringArmComponent> CameraString;

    // 실제 카메라
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UCameraComponent> MainCamera;

// 공격 기능 함수/변수 -------------------
protected:
    // 상속받은 TryAttack 오버라이드
    virtual void TryAttack() override;
    
    // 상속받은 Attack_Implemetation 함수 구현
    virtual void Attack_Implementation() override;

    // 공격 대상을 구분하는 함수(적) 구체화
    virtual bool CanAttackTarget(AActor* Target) const;

    // HitStop(역경직) 적용 시간
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float HitStopTime = 0.05f;

    // HitStop(역경직) 및 피격 시 월드 타이머 짧게 중단
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ApplyHitStop(float time);

    // AttackDelay 적용 함수
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void PlayerAttackDelay(float Time);

    // 공격 딜레이 후 다시 공격이 가능하도록 전환
    void PlayerAttackDelayReset();

    // 공격 시 전진 거리
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float AttackStepForce = 500.f;

    // 달리는 도중 공격 시 전진거리 배율
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    float AttackStepForceMultiplierWhileRun = 2.0f;

    // 공격 시작 전 당시의 속도 저장 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    float SavedAttackSpeed;

// 플레이어 콤보 공격 관련 -------------------
protected:
    // 플레이어 콤보 연계가 가능하도록 전환 (ABP의 노티파이에서 호출함)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CheckCombo();

    // 플레이어의 공격 종료 신호 전달 함수 (ABP의 노티파이에서 호출함)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ResetCombo();

    // 현재 플레이어의 공격 타수
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    int32 ComboCount = 0;

    // 공격 연계 타이밍 이전의 연계 입력은 무시하기 위한 트리거
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bIgnoreSaveAttack = true;

    // 공격 도중 버튼을 또 눌렀는지 판단, 애니메이션 노티파이와 함께 사용(AS_Player), 애니메이션 도중의 입력 판단에 사용됩니다. 
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    bool bSaveAttack = false; 

    // 첫 번째 공격 이후 몇초간 콤보를 이어갈 입력을 받는지 정의
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float SecondAttackWaitTime = 0.2f;

    // 첫 번째 공격 이후 공격 대기상태 판단, 애니메이션 종료 후 입력 판단에 사용됩니다. 
    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    bool bIsWaitNextAttackInput = false;

    // 일반공격/피격 이후 상태 복구 함수 (부모 클래스 함수 재사용)
    virtual void EndAttackState() override;

private:
    // 콤보 초기화 변수가 포함된 함수
    void FullResetCombo();

// 피격 관련 함수/변수
protected:
    // 부모클래스에서 상속받아 사용
    virtual void GetHit(const FDamageData& DamageData) override;

    // 플레이어 피격무적 시간 변수
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    float HitInvincibleTime = 1.0f;

// 플레이어 이동 관련 함수/변수
protected:
    // 점프 시도
    UFUNCTION(BlueprintCallable, Category = "Player_Movement")
    virtual void TryJump();

    // 점프 중단(점프 버튼을 손에서 뗏을 때)
    UFUNCTION(BlueprintCallable, Category = "Player_Movement")
    virtual void TryStopJumping();

    // 플레이어 최대 점프 가능 횟수, 근데 이거 어차피 CharacterMovement 클래스에 있는 변수라 안쓸 수 있음
    UPROPERTY(EditDefaultsOnly, Category = "Player_Movement")
    int32 MaxJumpCount = 1;

    // 애니메이션 이벤트에서 호출할 전진 스텝 함수 
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void StepForward(float StepForce = 800.0f);


// 기타 추가 기능
protected:
    // 플레이어가 이동/공격을 제한하는 이상상태에 있는지 판단하는 함수
    // 행동 가능하면 true, 행동이 불가능하면 false를 반환
    virtual bool IsCharacterCanAction() override;

    // 플레이어 공격 관련 타이머 관리자
    FTimerHandle PlayerTimerHandle;

    // 플레이어 콤보 관련 타이머 관리자
    FTimerHandle ComboTimerHandle;

    // 플레이어 피격 관련 타이머 관리자
    FTimerHandle HitInvincibleTimerHandle;
    
};

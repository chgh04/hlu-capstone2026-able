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
    UPROPERTY(EditAnywhere, Category = "Combat")
    float HitStopTime = 0.05f;

// 플레이어 콤보 공격 관련 -------------------
protected:
    // 플레이어 콤보 연계가 가능하도록 전환 (ABP의 노티파이에서 호출함)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void CheckCombo();

    // 플레이어의 공격 종료 신호 전달 함수 (ABP의 노티파이에서 호출함)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ResetCombo();

    // 현재 타수
    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    int32 ComboCount = 0;

    // 공격 연계 타이밍 이전의 연계 입력은 무시하기 위한 트리거
    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    bool bIgnoreSaveAttack = true;

    // 공격 도중 버튼을 또 눌렀는지 판단, 애니메이션 노티파이와 함께 사용(AS_Player)
    UPROPERTY(BlueprintReadWrite, Category = "Combat")
    bool bSaveAttack = false; 
// 플레이어 이동 관련 함수/변수
protected:
    // 점프 시도
    UFUNCTION(BlueprintCallable, Category = "Player_Movement")
    virtual void TryJump();

    // 점프 중단(점프 버튼을 손에서 뗏을 때)
    UFUNCTION(BlueprintCallable, Category = "Player_Movement")
    virtual void TryStopJumping();

    UPROPERTY(EditAnywhere, Category = "Player_Movement")
    int32 MaxJumpCount = 1;

// 기타 추가 기능
    
};


#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "BossEnemyBase.generated.h"

/**
 * 보스몬스터의 핵심 클래스, 상태 컴포넌트와 FSM 컴포넌트를 통해 동작
 * 
 */
UCLASS()
class HLU_CAPSTONE_2026_API ABossEnemyBase : public AEnemyBase
{
	GENERATED_BODY()

public:
	ABossEnemyBase();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

// 컴포넌트 생성
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBossStatComponent* BossStatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UBossFSMComponent* BossFSMComponent;

// 보스 피해 관련 함수/변수
protected:
	// 부모 GetHit 함수 오버라이드
	virtual bool GetHit(const FDamageData& DamageData) override;

	// 보스 사망시 호출될 함수
	virtual void OnDeath_Implementation() override;

// 그로기 관련 함수/변수
protected:
	// 스탯 컴포넌트의 델리게이트를 받는 함수 
	UFUNCTION()
	void HandleGroggyStateChange(bool bIsGroggy);

	// 그로기 애니메이션/이펙트 시작시 호출 함수, 넉백시에만 호출되는 HitAnimation과는 별도로 구성
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy_Boss_Visuals")
	void OnBossGroggyStarted();

	// 그로기 애니메이션/이펙트 종료시 호출 함수
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy_Boss_Visuals")
	void OnBossGroggyEnded();

// 보스 AI 관련 함수/변수
public:
	// 타겟 고정 함수
	UFUNCTION(BlueprintCallable, Category = "Enemy_Boss_AI")
	void SetBossTarget();

	// 컴포넌트에서 사용할 수 있도록 타겟 반환
	AActor* GetBossTarget() const { return FixedTargetPlayer; }

	// FSM이 결정한 이벤트 호출, 블루프린트에서 Switch On Name으로 애니메이션 오버라이드 재생 
	UFUNCTION(BlueprintImplementableEvent, Category = "Enemy_Boss_AI")
	void ExecuteBossPattern(FName PatternName);

	// 캐릭터 행동이 종료되었을 때 호출됨, 변수 등 초기화
	UFUNCTION(BlueprintCallable, Category = "Enemy_Boss_AI")
	void BossActionEnd();

	// 고정된 타겟 반환 함수
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Enemy_Boss_AI")
	AActor* GetFixedTargetPlayer() const { return FixedTargetPlayer; }

protected:
	// 한 번 인식한 플레이어를 영구적으로 기억할 포인터
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy_Boss_AI")
	AActor* FixedTargetPlayer = nullptr;
};


#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BossStatComponent.generated.h"

/*
보스의 상태, 그로기, 페이즈를 관리합니다. 
*/

// FSM 상태 전환을 위한 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGroggyGaugeChangedSignature, float, CurrentGroggy, float, MaxGroggy);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGroggyStateChangedSignature, bool, bIsGroggy);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HLU_CAPSTONE_2026_API UBossStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBossStatComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

// 델리게이트 선언 
public: 
	UPROPERTY(BlueprintAssignable, Category = "Boss_Events")
	FOnGroggyGaugeChangedSignature OnGroggyGaugeChanged;

	UPROPERTY(BlueprintAssignable, Category = "Boss_Events")
	FOnGroggyStateChangedSignature OnGroggyStateChanged;

// 보스 피해 관련 함수/변수
public:
	// ABossEnemyBase에서 데미지를 받을 때 호출할 함수
	UFUNCTION(BlueprintCallable, Category = "Boss_Stats")
	void ApplyDamage(float DamageAmount);

	// 보스가 그로기 상태 플래그 반환
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Boss_Stats")
	bool GetIsGroggy() const { return bIsGroggy; }

private:
	// 그로기 상태를 끝내고 게이지 초기화
	void RecoverFromGroggy();
		
// 보스 전용 스탯 변수
protected:
	// 그로기 게이지 최댓값
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy_Boss_Stats")
	float MaxGroggy = 20.0f;

	// 현재 그로기 게이지 
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy_Boss_Stats")
	float CurrentGroggy;

	// 그로기 상태 유지 시간 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy_Boss_Stats")
	float GroggyDuration = 4.0f;

	// 현재 그로기 상태인지에 대한 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enemy_Boss_Stats")
	bool bIsGroggy = false;

// 기타 기능
protected:
	// 그로기 회복 타이머
	FTimerHandle GroggyRecoveryTimerHandle;
};

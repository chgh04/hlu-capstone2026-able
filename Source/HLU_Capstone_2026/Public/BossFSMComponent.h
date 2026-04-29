
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BossFSMComponent.generated.h"

/*
보스몬스터의 행동 규칙(FSM)을 정의합니다.
보스의 공격/행동은 열거형과 구조체로 정의합니다. 
*/

/*	***반복 행동 선택에 대한 설계***
	W_f = W_b * gamma^(N)
	(최종 행동 선택 확률) = (행동 기본 가중치) * (반복 패널티 계수)^(해당 카테고리/패턴의 연속 선택 횟수)
	예를 들어, 물기의 공격 확률이 50%이고 물기 공격이 시행되면, 다음 물기 공격의 확률은 25%가 됨
*/

// 보스 행동 카테고리
UENUM(BlueprintType)
enum class EBossActionCategory : uint8
{
	Attack  UMETA(DisplayName = "Attack"),
	Move    UMETA(DisplayName = "Move"),
	Guard   UMETA(DisplayName = "Guard"),
	Dodge   UMETA(DisplayName = "Dodge"),
	Special UMETA(DisplayName = "Special")
};

// 보스 FSM 열거형
UENUM(BlueprintType)
enum class EBossFSMState : uint8
{
	Idle            UMETA(DisplayName = "Idle"),
	Moving			UMETA(DisplayName = "Moving"),
	ExecutingAction UMETA(DisplayName = "Executing Action"),
	Groggy          UMETA(DisplayName = "Groggy"),
	Dead            UMETA(DisplayName = "Dead")
};


// 보스 패턴 데이터 구조체
USTRUCT(BlueprintType)
struct FBossPattern
{
	GENERATED_BODY()

public:
	// 패턴 고유 이름 (예: "Bite", "JumpBack") - 나중에 BP나 애니메이션 실행 트리거로 사용
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName PatternName;

	// 카테고리 (분류용)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBossActionCategory Category = EBossActionCategory::Attack;

	// 패턴에 대한 선택 기본 가중치 (점수)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float BaseWeight = 50.0f;

	// 이 패턴을 사용하기 위한 플레이어와의 최소 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MinDistance = 0.0f;

	// 이 패턴을 사용하기 위한 플레이어와의 최대 거리
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDistance = 1500.0f;

	// 해당 쿨타임 (연속 사용 방지)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Cooldown = 3.0f;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HLU_CAPSTONE_2026_API UBossFSMComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBossFSMComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// 클래스 본체
	class ABossEnemyBase* BossOwner;

// 보스 패턴 관리 함수/변수 --------------------------------------
public:
	// 에디터에 노출할 보스의 사용 가능 패턴 목록
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy_Boss_Pattern")
	TArray<FBossPattern> AvailablePatterns;

	// 행동 선택 로직, 선택에 대한 2차 확률선택 (패턴고리 선택)
	UFUNCTION(BlueprintCallable, Category = "Enemy_Boss_Pattern")
	FName SelectNextPattern(float DistanceToPlayer);

	// 모든 행동 기록 초기화
	UFUNCTION(BlueprintCallable, Category = "Enemy_Boss_Pattern")
	void ResetActionHistory();

private:
	// 카테고리의 선택 횟수 추적
	TMap<EBossActionCategory, int32> CategoryUsageHistory;

	// 카테고리 내부의 행동 선택 횟수 추적
	TMap<FName, int32> PatternUsageHistory;

	// 행동 선택에 대한 1차 확률선택 (카테고리 선택)
	EBossActionCategory EvaluateCategories(float DistanceToPlayer);

	// 행동 기록 업데이트 및 리셋 검사 함수
	void UpdateActionHistory(EBossActionCategory Category, FName PatternName);

	// 카테고리별 반복 패널티 계수
	UPROPERTY(EditAnywhere, Category = "Enemy_Boss_Pattern")
	float CategoryDecayFactor = 0.5f;

	// 패턴별 반복 페널티 계수
	UPROPERTY(EditAnywhere, Category = "Enemy_Boss_Pattern")
	float PatternDecayFactor = 0.5f;

	// 패턴별 사용시간 기록
	TMap<FName, float> PatternLastUsedTimes;

	// 모든 카테고리/행동패턴 초기화 횟수
	int32 ActionResetThreshold = 5;

	// 현재 보스가 수행한 총 행동 횟수
	int32 TotalActionCount = 0;

// 보스 FSM 관련 함수/변수 --------------------------------------
public:
	// FSM 시작(보스전 시작과 함께 호출)
	UFUNCTION(BlueprintCallable, Category = "Enemy_Boss_FSM")
	void StartFSM();
	
	// 행동 후 대기상태로 진입할 때 호출
	UFUNCTION(BlueprintCallable, Category = "Enemy_Boss_FSM")
	void EnterIdleState();

	// 행동 종료 알림 함수(애니메이션 노티파이에서 호출 혹은 애니메이션 오버라이드의 OnComplete에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Enemy_Boss_FSM")
	void ActionFinished();

	// 보스의 행동 사이 최소 대기 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy_Boss_FSM")
	float MinIdleTime = 1.0f;

	// 보스의 행동 사이 최대 대기 시간 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Enemy_Boss_FSM")
	float MaxIdleTime = 2.0f;

	// 외부에서 그로기 상태를 강제로 켜고 끄는 함수
	UFUNCTION(BlueprintCallable, Category = "Enemy_Boss_FSM")
	void SetGroggyState(bool bIsGroggy);

private:
	// 현재 보스의 상태
	UPROPERTY(VisibleAnywhere, Category = "Enemy_Boss_FSM")
	EBossFSMState CurrentState = EBossFSMState::Idle;

	// 대기 타이머 핸들
	FTimerHandle IdleWaitTimerHandle;

	// 다음 행동을 결정하는 함수
	void DecideNextAction();

	// 구조체 원본 전달 함수
	const FBossPattern* GetPatternDetails(FName PatternName);

	// 틱 기반 이동처리함수 
	void ChaseOnBossFSM();

	// 이동할 목표 거리 
	float CurrentMoveTargetDistance = 0.0f;

	// 타겟 방향 회전 함수
	

};

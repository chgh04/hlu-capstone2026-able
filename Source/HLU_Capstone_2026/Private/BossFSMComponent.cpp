
#include "BossFSMComponent.h"
#include "BossEnemyBase.h"

// Sets default values for this component's properties
UBossFSMComponent::UBossFSMComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UBossFSMComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// 본체 클래스 할당
	BossOwner = Cast<ABossEnemyBase>(GetOwner());
}

void UBossFSMComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	switch (CurrentState)
	{
		case EBossFSMState::Idle:
			// 대기상태일땐 아무것도 안함
			break;
			
		case EBossFSMState::Moving:
			// 이동중엔 플레이어를 바라보며 이동, 가까워지면 중단
			ChaseOnBossFSM();
			break;
			
		case EBossFSMState::ExecutingAction:
			// 행동 실행
			break;
			
		case EBossFSMState::Groggy:
			break;
		
		case EBossFSMState::Dead:
			return;
	}

	// 스프라이트 정렬
	BossOwner->ApplySpriteSortAmount();
}

FName UBossFSMComponent::SelectNextPattern(float DistanceToPlayer)
{
	// 1. 1차 카테고리 당첨 결과 가져오기
	EBossActionCategory ChosenCategory = EvaluateCategories(DistanceToPlayer);

	// 사용 가능 패턴을 저장할 맵
	TMap<FName, float> ValidPatterns;
	float TotalWeight = 0.0f;
	float CurrentTime = GetWorld()->GetTimeSeconds();

	// 2. 2차 선택 가능 패턴 후보 필터링 및 가중치 계산
	for (const FBossPattern& Pattern : AvailablePatterns)
	{	
		// 1차에서 선택된 카테고리가 아니라면 탈락 
		if (Pattern.Category != ChosenCategory)
		{
			continue;
		}

		// 거리 조건 탈락
		if (DistanceToPlayer < Pattern.MinDistance || DistanceToPlayer > Pattern.MaxDistance)
		{
			continue;
		}

		// 쿨타임 조건 탈락
		if (const float* LastUsedTime = PatternLastUsedTimes.Find(Pattern.PatternName))
		{	
			// 마지막으로 사용되었을때의 시간과 현재 시간의 차이가 쿨타임보다 적다면 탈락
			if (CurrentTime - *LastUsedTime < Pattern.Cooldown)
			{
				continue;
			}
		}

		// 패턴별 패널티 적용
		const int32* PtrUsageCount = PatternUsageHistory.Find(Pattern.PatternName);
		int32 UsageCount = PtrUsageCount ? *PtrUsageCount : 0;
		float FinalWeight = Pattern.BaseWeight * FMath::Pow(PatternDecayFactor, UsageCount);	// 누적된 값(CurrentWeight) + 해당 패턴의 가중치 합

		// 유효 패턴 목록에 추가 
		ValidPatterns.Add(Pattern.PatternName, FinalWeight);
		TotalWeight += FinalWeight;
	}

	// 3. 예외처리, 쓸수 있는 패턴이 없다면 아무것도 하지 않음
	if (ValidPatterns.Num() == 0 || TotalWeight <= 0.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss AI: No valid patterns in category! Waiting..."));
		return NAME_None;
	}

	// 4. 2차 룰렛, 룰렛 휠 선택 알고리즘
	float RouletteBall = FMath::FRandRange(0.0f, TotalWeight);
	float AccumulatedWeight = 0.0f;
	FName SelectedPattern = NAME_None;

	for (const auto& Pair : ValidPatterns)
	{
		AccumulatedWeight += Pair.Value;
		if (RouletteBall <= AccumulatedWeight)
		{
			SelectedPattern = Pair.Key;
			break;
		}
	}

	// 부동소수점 오차로 당첨이 안 된 경우 첫 번째 요소 강제 선택
	if (SelectedPattern == NAME_None)
	{
		SelectedPattern = ValidPatterns.CreateConstIterator().Key();
	}

	// 5. 결정된 패턴을 기록하고, 쿨타임 타이머 기록
	UpdateActionHistory(ChosenCategory, SelectedPattern);
	PatternLastUsedTimes.Add(SelectedPattern, CurrentTime);

	UE_LOG(LogTemp, Warning, TEXT("Boss AI: Selected Pattern -> [%s]"), *SelectedPattern.ToString());
	return SelectedPattern;
}

void UBossFSMComponent::ResetActionHistory()
{
	CategoryUsageHistory.Empty();
	PatternUsageHistory.Empty();
	TotalActionCount = 0;
	UE_LOG(LogTemp, Warning, TEXT("Boss AI History Reset!"));
}

EBossActionCategory UBossFSMComponent::EvaluateCategories(float DistanceToPlayer)
{
	// 1. 카테고리별 누적 가중치를 저장할 임시 맵
	TMap<EBossActionCategory, float> CategoryWeights;
	float CurrentTime = GetWorld()->GetTimeSeconds();

	// 2. 사용 가능 패턴 검사(모든 패턴 순회하여 사용 가능한 경우만 카테고리 점수 누적)
	for (const FBossPattern& Pattern : AvailablePatterns)
	{	
		// 특수패턴은 랜덤 호출 대상에서 제외
		if (Pattern.Category == EBossActionCategory::Special)
		{
			continue;
		}

		// 패턴의 거리조건 미달시 탈락
		if (DistanceToPlayer < Pattern.MinDistance || DistanceToPlayer > Pattern.MaxDistance)
		{
			continue;
		}

		// 패턴이 쿨타임 대기중일경우 탈락
		if (const float *LastUsedTime = PatternLastUsedTimes.Find(Pattern.PatternName))
		{
			if (CurrentTime - *LastUsedTime < Pattern.Cooldown)
			{
				continue;
			}
		}

		// 패턴이 유효하다면, 해당 카테고리 가중치에 패턴의 BaseWeight 누적
		const float* CtrCurrentWeight = CategoryWeights.Find(Pattern.Category);
		float CurrentWeight = CtrCurrentWeight ? *CtrCurrentWeight : 0.0f;
		CategoryWeights.Add(Pattern.Category, CurrentWeight + Pattern.BaseWeight);	// 누적된 값(CurrentWeight) + 해당 패턴의 가중치 합
	}

	// 3. 카테고리별 최종 가중치 계산 및 패널티 적용
	float TotalWeight = 0.0f;
	TMap<EBossActionCategory, float> FinalCategoryWeights;

	// 누적 가중치를 저장한 임시 맵 순회
	for (const auto& Pair : CategoryWeights)
	{	
		// 임시 맵의 키와 밸류(카테고리, 가중치) 저장
		EBossActionCategory Category = Pair.Key;
		float BaseWeight = Pair.Value;

		// 만약 카테고리가 카테고리 사용 기록에 포함되어 있다면 사용 횟수, 없다면 0(패널티 없음)을 기록
		int32 UseageCount = CategoryUsageHistory.Contains(Category) ? CategoryUsageHistory[Category] : 0;

		// 수학적 패널티 적용, 최종 카테고리 선택 확률 = 카테고리 선택확률 * (패널티 계수 ^ 패턴사용횟수)
		float FinalWeight = BaseWeight * FMath::Pow(CategoryDecayFactor, UseageCount);

		// 최종 카테고리별 가중치 맵에 카테고리와 해당 카테고리의 가중치를 함께 저장
		FinalCategoryWeights.Add(Category, FinalWeight);
		TotalWeight += FinalWeight;
	}

	// 4. 만약 거리가 멀거나 쿨타임때문에 하나도 쓸 패턴이 없을때 
	if (TotalWeight <= 0.0f)
	{	
		// 이동 카테고리로 강제 반환
		UE_LOG(LogTemp, Warning, TEXT("Boss AI: No available categories! Defaulting to Move."));
		return EBossActionCategory::Move;
	}

	// 5. 가중치 기반 룰렛, 0부터 가중치 총합 사이의 무작위 난수 구하기
	float RouletteBall = FMath::FRandRange(0.0f, TotalWeight);
	float AccumulatedWeight = 0.0f;

	for (const auto& Pair : FinalCategoryWeights)
	{
		AccumulatedWeight += Pair.Value;

		if (RouletteBall <= AccumulatedWeight)
		{
			UE_LOG(LogTemp, Warning, TEXT("Boss AI: Category %d selected with Weight %f"), (int32)Pair.Key, Pair.Value);
			return Pair.Key;
		}
		/*
		룰렛 휠 선택 알고리즘
		-> 각 행동마다의 가중치가 10, 20, 30, 40이 있고, 모든 합이 100일때 0~100 사이의 값을 무작위하게 뽑고
		-> 각 카테고리의 가중치와 비교하며 10보다 작은가? 30(10+20)보다 작은가? 60(10+20+30)보다 작은가? 100(10+20+30+40)보다 작은가?를 비교
		-> 만약 65의 값이 뽑혔다면 10, 30, 60보다 크지만 100보다 작으므로 가중치가 40인 카테고리/행동이 선택되는것
		*/
	}

	// 컴파일러 경고 방지용 카테고리 반환
	return EBossActionCategory::Move;
}

void UBossFSMComponent::UpdateActionHistory(EBossActionCategory Category, FName PatternName)
{
	// 카테고리 및 패턴 사용 횟수 1 증가
	int32& CatCount = CategoryUsageHistory.FindOrAdd(Category);
	CatCount++;

	int32& PatCount = PatternUsageHistory.FindOrAdd(PatternName);
	PatCount++;

	TotalActionCount++;

	// 임계치에 도달할경우 패널티 리셋
	if (TotalActionCount >= ActionResetThreshold)
	{
		ResetActionHistory();
	}
}

void UBossFSMComponent::StartFSM()
{	
	// 플레이어를 고정된 목표로 설정
	BossOwner->SetBossTarget();

	// 보스전 시작, 처음엔 짧게 대기 후 공격 시작
	EnterIdleState();
}

void UBossFSMComponent::EnterIdleState()
{
	// 이미 죽었다면 대기 상태로 가지 않음
	if (CurrentState == EBossFSMState::Dead)
	{
		return;
	}

	CurrentState = EBossFSMState::Idle;

	// 행동간 보스의 대기시간 지정
	float WaitTime = FMath::FRandRange(MinIdleTime, MaxIdleTime);

	// 대기시간동안 아무 행동을 하지 않음(Idle 상태)
	GetWorld()->GetTimerManager().SetTimer(
		IdleWaitTimerHandle,
		this,
		&UBossFSMComponent::DecideNextAction,
		WaitTime,
		false
	);

	UE_LOG(LogTemp, Warning, TEXT("Boss AI: Enter IDLE. Waiting for %f seconds..."), WaitTime);

}

void UBossFSMComponent::SetGroggyState(bool bIsGroggy)
{
	if (bIsGroggy)
	{
		// 상태를 즉시 그로기로 덮어씌움
		CurrentState = EBossFSMState::Groggy;

		// 대기 중이던 타이머가 있다면 즉시 취소 (그로기 도중 행동 실행 방지)
		GetWorld()->GetTimerManager().ClearTimer(IdleWaitTimerHandle);

		UE_LOG(LogTemp, Warning, TEXT("Boss AI: Forced into GROGGY state! All FSM timers stopped."));
	}
	else
	{
		// 그로기가 끝났으므로 다시 대기 상태로 돌려보내 사이클 재시작
		UE_LOG(LogTemp, Warning, TEXT("Boss AI: Groggy over. Restarting FSM cycle."));
		EnterIdleState();
	}
}

void UBossFSMComponent::DecideNextAction()
{	
	// 만약 대기중인 아닐때 호출되었다면 리턴 
	if (CurrentState != EBossFSMState::Idle)
	{
		return;
	}

	float Distance = 0.0f;

	if (BossOwner && BossOwner->GetBossTarget())
	{
		// 플레이어와의 거리 계산
		Distance = FVector::Dist(BossOwner->GetActorLocation(), BossOwner->GetBossTarget()->GetActorLocation());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Boss AI: No Fixed Target! return FSM"));
		return;
	}

	// 행동 선택 로직을 거리를 주고 호출
	FName NextPatternName = SelectNextPattern(Distance);

	// 엣지 케이스 처리 (쓸 패턴이 아무것도 없으면 다시 짧게 대기)
	if (NextPatternName == NAME_None)
	{
		EnterIdleState();
		return;
	}

	// 플레이어 방향 보기
	FVector LookDirection = BossOwner->GetBossTarget()->GetActorLocation() - BossOwner->GetActorLocation();
	LookDirection.Z = 0.0f;

	if (!LookDirection.IsNearlyZero())
	{
		BossOwner->SetActorRotation(LookDirection.Rotation());
	}

	// 선택된 패턴의 구조체 가져오기
	const FBossPattern* ChosenPattern = GetPatternDetails(NextPatternName);
	if (!ChosenPattern) return;

	// 이동 패턴의 경우 Moving 상태로 전환 
	if (ChosenPattern->Category == EBossActionCategory::Move)
	{
		CurrentState = EBossFSMState::Moving;
		CurrentMoveTargetDistance = ChosenPattern->MinDistance;	// 목표 거리 설정(이동패턴의 최소거리)

		UE_LOG(LogTemp, Warning, TEXT("Boss AI: Start Moving towards Player!"));
	}
	else // 이동 이외의 행동일 경우
	{	
		// 행동 실행 상태로 전환
		CurrentState = EBossFSMState::ExecutingAction;

		// 본체 클래스에게 애니메이션 재생 명령 전달
		BossOwner->ExecuteBossPattern(NextPatternName);
	}
}

const FBossPattern* UBossFSMComponent::GetPatternDetails(FName PatternName)
{
	for (const FBossPattern& Pattern : AvailablePatterns)
	{
		if (Pattern.PatternName == PatternName)
			return &Pattern;
	}
	return nullptr;
}

void UBossFSMComponent::ActionFinished()
{
	if (CurrentState == EBossFSMState::ExecutingAction)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss AI: Action Finished (By Anim Notify). Back to Idle."));

		// 상태 초기화 함수 호출
		BossOwner->BossActionEnd();

		// 행동이 끝났으니 다시 대기(호흡) 상태로 돌아감
		EnterIdleState();
	}
}

void UBossFSMComponent::ChaseOnBossFSM()
{	
	if (!BossOwner->GetBossTarget())
	{
		return;
	}

	float Distance = FVector::Dist(BossOwner->GetActorLocation(), BossOwner->GetBossTarget()->GetActorLocation());

	// 목표거리와 가깝다면 이동 종료, 오차범위 50.0f
	if (Distance <= CurrentMoveTargetDistance + 50.0f)
	{
		UE_LOG(LogTemp, Warning, TEXT("Boss AI: Target Reached! Back to Idle."));

		// 몽타주 노티파이가 없으므로, 여기서 스스로 대기 상태로 루프 넘기기
		EnterIdleState();
		return;
	}

	// 멀다면 플레이어 방향으로 이동입력 
	FVector Direction = (BossOwner->GetBossTarget()->GetActorLocation() - BossOwner->GetActorLocation());

	// 수평이동만 적용
	Direction.Z = 0.0f;

	BossOwner->AddMovementInput(Direction, 1.0f);
}


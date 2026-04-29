
#include "BossEnemyBase.h"
#include "BossStatComponent.h"
#include "BossFSMComponent.h"
#include "HealthComponent.h"
#include "Components/BoxComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

ABossEnemyBase::ABossEnemyBase()
{
	// 보스 전용 스탯 컴포넌트 생성
	BossStatComponent = CreateDefaultSubobject<UBossStatComponent>(TEXT("BossStatComponent"));

	// 보스 전용 FSM 컴포넌트 생성
	BossFSMComponent = CreateDefaultSubobject<UBossFSMComponent>(TEXT("BossFSMComponent"));

	// 넉백 면역 설정
	bIsKnockBackImmune = true;

	// SimpleFSM 미사용 설정
	bUseSimpleFSM = false;
}

void ABossEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (BossStatComponent)
	{
		// 그로기 상태 변화 델리게이트에 바인딩
		BossStatComponent->OnGroggyStateChanged.AddDynamic(this, &ABossEnemyBase::HandleGroggyStateChange);
	}
}

void ABossEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

bool ABossEnemyBase::GetHit(const FDamageData& DamageData)
{
	// 부모의 기본 피격 로직 먼저 실행
	// 무적/가드/회피 판정 및 
	if (!Super::GetHit(DamageData))
	{
		return false;
	}

	// 보스 스탯 컴포넌트에 그로기 게이지 증가 요청
	if (BossStatComponent && !BossStatComponent->GetIsGroggy())
	{
		BossStatComponent->ApplyDamage(DamageData.DamageAmount);
	}

	return true;
}

void ABossEnemyBase::HandleGroggyStateChange(bool bIsGroggy)
{
	if (bIsGroggy)
	{
		// 그로기 시작: 모든 행동 중단 및 이동 불가 처리
		GetCharacterMovement()->StopMovementImmediately();
		bIsKnockBack = true; // 이동 제한 플래그 활용

		// FSM 상태머신에 강제 그로기상태 주입
		if (BossFSMComponent)
		{
			BossFSMComponent->SetGroggyState(true);
		}

		// 상태 초기화 함수 호출
		BossActionEnd();

		// 블루프린트에 알림
		OnBossGroggyStarted();

		UE_LOG(LogTemp, Warning, TEXT("Boss: Groggy Animation Triggered"));
	}
	else
	{
		// 그로기 종료: 다시 행동 가능 상태로 복구
		bIsKnockBack = false;

		// FSM 상태머신에 그로기 해제
		if (BossFSMComponent)
		{
			BossFSMComponent->SetGroggyState(false);
		}

		OnBossGroggyEnded();

		UE_LOG(LogTemp, Warning, TEXT("Boss: Groggy Over, Returning to Battle"));
	}
}

void ABossEnemyBase::OnDeath_Implementation()
{	
	// 보스의 사망로직은 EnemyBase와는 별개로 전용 이벤트를 구현하기 위해 부모로직을 실행하지 않음 

	UE_LOG(LogTemp, Warning, TEXT("EnemyBossBase: Boss Killed!"));

	// 플래그 전환
	if (!bIsDead)
	{
		bIsDead = true;
	}

	// 사망 애니메이션 재생
	PlayDeathAnimation();

	// 공격 판정 박스 끄기
	if (AttackBox)
	{
		AttackBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 추후 효과 추가
}

void ABossEnemyBase::SetBossTarget()
{
	if (FixedTargetPlayer != nullptr)
	{	
		// 이미 타겟이 있다면 무시 
		return;
	}

	if (TargetPlayer)
	{
		FixedTargetPlayer = TargetPlayer;
		UE_LOG(LogTemp, Warning, TEXT("Boss Target Locked: %s"), *FixedTargetPlayer->GetName());
	}
}

void ABossEnemyBase::BossActionEnd()
{
	// 지면마찰력 재적용
	if (MovementComp)
	{
		MovementComp->GroundFriction = SavedGroundFriction;
	}
	
	// 공격 판정 한 번 더 제거(노티파이를 사용하지 않는 경우 여기서만 삭제함)
	EndAttackCollision();

	// 회피플래그 초기화
	bIsDodging = false;

	// 공격플래그 초기화
	bIsAttacking = false;
}
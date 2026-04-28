
#include "BossStatComponent.h"

UBossStatComponent::UBossStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}


void UBossStatComponent::BeginPlay()
{
	Super::BeginPlay();

	CurrentGroggy = 0.0f;
	bIsGroggy = false;
}

void UBossStatComponent::ApplyDamage(float DamageAmount)
{	
	// 그로기 게이지 증가 
	if (!bIsGroggy)
	{	
		// 게이지 증가
		CurrentGroggy = FMath::Clamp(CurrentGroggy + DamageAmount, 0.0f, MaxGroggy);

		UE_LOG(LogTemp, Warning, TEXT("Boss: Groggy Added!, Now: %.2f"), CurrentGroggy);

		// 게이지 증가 브로드케스팅 
		OnGroggyGaugeChanged.Broadcast(CurrentGroggy, MaxGroggy);

		if (CurrentGroggy >= MaxGroggy)
		{
			// 플래그 전환
			bIsGroggy = true;

			OnGroggyStateChanged.Broadcast(true);

			UE_LOG(LogTemp, Warning, TEXT("Boss is now in GROGGY STATE!"));

			// 지정된 시간(GroggyDuration) 이후에 RecoverFromGroggy 함수 실행
			GetWorld()->GetTimerManager().SetTimer(
				GroggyRecoveryTimerHandle,
				this,
				&UBossStatComponent::RecoverFromGroggy,
				GroggyDuration,
				false
			);
		}
	}
}

void UBossStatComponent::RecoverFromGroggy()
{
	// 그로기 상태 해제 및 게이지 0으로 초기화
	bIsGroggy = false;
	CurrentGroggy = 0.0f;

	// FSM에게 그로기 종료 브로드캐스팅 
	OnGroggyStateChanged.Broadcast(false);
	OnGroggyGaugeChanged.Broadcast(CurrentGroggy, MaxGroggy);

	UE_LOG(LogTemp, Warning, TEXT("Boss recovered from Groggy. Ready to fight!"));
}


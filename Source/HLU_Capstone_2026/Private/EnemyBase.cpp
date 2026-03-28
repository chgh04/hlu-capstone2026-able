#include "EnemyBase.h"
#include "HealthComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyBase::AEnemyBase()
{
    // AI 이동/감지는 타이머로 처리하므로 Tick 불필요 - 성능 최적화
    PrimaryActorTick.bCanEverTick = false;

    // 플레이어 감지 구 생성 - RootComponent에 부착
    DetectionRange = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionRange"));
    DetectionRange->SetupAttachment(RootComponent);
    DetectionRange->SetSphereRadius(DetectionRadius);
    DetectionRange->SetCollisionProfileName(TEXT("Trigger")); // Overlap만 감지
}

void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();

    // 1초마다 플레이어 감지 타이머 시작, bLoop = true로 반복 실행
    GetWorldTimerManager().SetTimer(
        DetectionTimerHandle,
        this,
        &AEnemyBase::DetectPlayer,
        1.0f,
        true
    );
}

void AEnemyBase::Attack_Implementation()
{
    Super::Attack_Implementation();

    UE_LOG(LogTemp, Warning, TEXT("Enemy is now attack"));
}

void AEnemyBase::DetectPlayer()
{
    // 플레이어 인덱스 0번 폰 가져오기 
    APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
    if (!PlayerPawn) return;

    // 플레이어까지의 거리 계산
    float Distance = FVector::Dist(GetActorLocation(), PlayerPawn->GetActorLocation());

    // 감지 반경 안에 있으면 저장, 밖이면 nullptr
    TargetPlayer = (Distance <= DetectionRadius) ? PlayerPawn : nullptr;
}


void AEnemyBase::OnDeath_Implementation()
{
    // 감지 타이머 정지 - 사망 후 계속 실행되는 것 방지
    GetWorldTimerManager().ClearTimer(DetectionTimerHandle);

    // 부모공통로직 실행
    Super::OnDeath_Implementation();
}
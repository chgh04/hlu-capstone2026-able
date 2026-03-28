#include "EnemyBase.h"
#include "HealthComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

AEnemyBase::AEnemyBase()
{
    // AI 이동/감지는 타이머로 처리하므로 Tick 불필요 - 성능 최적화
    PrimaryActorTick.bCanEverTick = false;

    // 체력 컴포넌트 생성 및 부착
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    // 플레이어 감지 구 생성 - RootComponent에 부착
    DetectionRange = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionRange"));
    DetectionRange->SetupAttachment(RootComponent);
    DetectionRange->SetSphereRadius(DetectionRadius);
    DetectionRange->SetCollisionProfileName(TEXT("Trigger")); // Overlap만 감지

    // 공격 판정 박스 생성 - RootComponent에 부착
    AttackBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackBox"));
    AttackBox->SetupAttachment(RootComponent);
    AttackBox->SetCollisionProfileName(TEXT("Trigger"));
    AttackBox->SetGenerateOverlapEvents(true);
}

void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();

    // 공격 박스 Overlap 이벤트 바인딩
    AttackBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::OnAttackBoxOverlap);

    // 1초마다 플레이어 감지 타이머 시작, bLoop = true로 반복 실행
    GetWorldTimerManager().SetTimer(
        DetectionTimerHandle,
        this,
        &AEnemyBase::DetectPlayer,
        1.0f,
        true
    );
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

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // 인터페이스 전달을 위한 구조체 선언 및 초기화
    FDamageData Data;
    Data.DamageAmount = DamageAmount;
    Data.DamageCauser = DamageCauser;
    Data.HitDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();

    // IDamageable::ReceiveDamage로 연결 후 데이터 전달
    if (this->Implements<UDamageable>())
    {
        IDamageable::Execute_ReceiveDamage(this, Data);
    }
    
    
    return DamageAmount;
}

void AEnemyBase::ReceiveDamage_Implementation(const FDamageData& DamageData)
{
    // HealthComponent가 유효하면 체력 감소 처리
    if (HealthComponent)
    {
        HealthComponent->ReduceHealth(DamageData.DamageAmount);
    }

    // TODO: 피격 방향 넉백, 시선 전환 등
}

void AEnemyBase::OnDeath_Implementation()
{
    // 감지 타이머 정지 - 사망 후 계속 실행되는 것 방지
    GetWorldTimerManager().ClearTimer(DetectionTimerHandle);

    // 블루프린트에서 override해서 사망 애니메이션, 이펙트, 아이템 드롭 후 Destroy 
    Destroy();
}

void AEnemyBase::Attack_Implementation()
{
    // 블루프린트에서 override해서 구현
    // 공격 애니메이션 재생, AttackBox 활성화 타이밍 조정 등
}

void AEnemyBase::OnAttackBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    // 현재는 미구현
    // BasePlayer 클래스 완성 후 아래처럼 데미지 적용 예정:
    // ABasePlayer* Player = Cast<ABasePlayer>(OtherActor);
    // if (Player) Player->ReceiveDamage(AttackDamage);
}
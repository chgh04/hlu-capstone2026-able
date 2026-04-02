#include "EnemyBase.h"
#include "HealthComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BlueprintGameplayTagLibrary.h"
#include "PlayerBase.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyBase::AEnemyBase()
{
    PrimaryActorTick.bCanEverTick = true;

    // 플레이어 감지 구 생성 - RootComponent에 부착
    DetectionRange = CreateDefaultSubobject<USphereComponent>(TEXT("DetectionRange"));
    DetectionRange->SetupAttachment(RootComponent);
    DetectionRange->SetSphereRadius(DetectionRadius);
    DetectionRange->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); // 모든 Collision 무시
    // 이거 안되면 BP에서 플레이어 채널만 overlap
    DetectionRange->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
}

void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();

    // Detection Range 감지 이벤트 바인딩
    if (DetectionRange)
    {
        DetectionRange->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::OnDetectionBeginOverlap);
        DetectionRange->OnComponentEndOverlap.AddDynamic(this, &AEnemyBase::OnDetectionEndOverlap);
        //UE_LOG(LogTemp, Warning, TEXT("AI: AddDynamic Added"));
    }

    // 상태 지정
    CurrentState = EEnemyState::Patrol;
}

void AEnemyBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bUseSimpleFSM) return;

    switch (CurrentState)
    {
    case EEnemyState::Patrol:
        //UE_LOG(LogTemp, Warning, TEXT("AI: Patrol"));
        // 순찰 로직 실행 (예: 무작위 위치로 이동), 기본상태
        break;

    case EEnemyState::Chase:
        //UE_LOG(LogTemp, Warning, TEXT("AI: Chase"));
        // TargetPlayer가 유효한지 확인 후 그쪽으로 이동
        if (TargetPlayer)
        {   
            // 추격 함수
            ChaseOnSimpleFSM();
        }
        break;

    case EEnemyState::Attack:
        if (bIsAttacking) break;
        //UE_LOG(LogTemp, Warning, TEXT("AI: Attack"));
        // 이동 멈춤 및 공격 실행
        TryAttack();
        // 공격이 끝나면 다시 거리를 재고 Chase로 돌아가는 로직은 각 캐릭터 BP에서 공격 애니메이션과 함께 구현 필요
        break;

    case EEnemyState::Hit:
        //UE_LOG(LogTemp, Warning, TEXT("AI: Hit"));
        // 넉백 중이므로 아무 행동도 하지 않고 대기 (GetHit 함수에서 타이머 실행됨)
        break;

    case EEnemyState::Dead:
        //UE_LOG(LogTemp, Warning, TEXT("AI: Dead"));
        // 이미 죽었으므로 아무것도 하지 않음

        OnDeath_Implementation();
        break;
    }
}

void AEnemyBase::Attack_Implementation()
{   
    //Super::Attack_Implementation();
    //UE_LOG(LogTemp, Warning, TEXT("Enemy is now attack"));
    
    bIsAttacking = true;
}

bool AEnemyBase::CanAttackTarget(AActor* Target) const
{   
    // 부모의 기본 체크(자기 자신 등)를 먼저 통과해야 함
    if (!Super::CanAttackTarget(Target)) return false;

    // 타겟의 태그 인터페이스를 불러오기
    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Target);

    // 태그 인터페이스가 없다면 false 
    if (!TagInterface)
    {
        return false;
    }

    FGameplayTag PlayerTag = FGameplayTag::RequestGameplayTag(FName("Entity.Team.Player"));

    // "Player" 태그를 가진 대상만 공격
    return TagInterface->HasMatchingGameplayTag(PlayerTag);
}

void AEnemyBase::GetHit(const FDamageData& DamageData)
{
    // 부모 로직 실행, 부모 로직에선 기본 넉백 적용
    // 이게 무슨코드더라? HealthComponent와 연결되어 피해를 받을 경우 피해 정보를 받아옴
    Super::GetHit(DamageData);

    // 현재 상태를 Hit으로 변겅
    CurrentState = EEnemyState::Hit;

    // 현재 캐릭터의 물리적 속도를 제거 (이건 선택사항)
    GetCharacterMovement()->StopMovementImmediately();

    // 공격이 불가능하도록 bIsAttacking을 True로 지정
    bIsAttacking = true;

    // 타이머 설정 및 타이머 이후 호출 함수 지정(ResetHitStateOnSimpleFSM)
    GetWorldTimerManager().ClearTimer(HitStunTimerHandle);
    GetWorldTimerManager().SetTimer(HitStunTimerHandle, this, &AEnemyBase::ResetHitStateOnSimpleFSM, StunDuration, false);
}

void AEnemyBase::OnDeath_Implementation()
{
    // 부모공통로직 실행
    Super::OnDeath_Implementation();

    CurrentState = EEnemyState::Dead;

    PrimaryActorTick.bCanEverTick = false;
}

void AEnemyBase::OnDetectionBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{   
    if (OtherActor && OtherActor != this)
    {
        APlayerBase* OverlappedPlayer = Cast<APlayerBase>(OtherActor);

        if (OverlappedPlayer)
        {
            TargetPlayer = OverlappedPlayer;
            //UE_LOG(LogTemp, Warning, TEXT("AI: Player Detected!"));

            // 추적 상태(State)로 전환
            CurrentState = EEnemyState::Chase;
        }
    }
}

void AEnemyBase::OnDetectionEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // 나간 액터가 현재 내가 타겟으로 삼고 있는 그 플레이어인지 확인
    if (OtherActor && OtherActor == TargetPlayer)
    {
        TargetPlayer = nullptr; // 타겟 비우기
        //UE_LOG(LogTemp, Warning, TEXT("AI: Player Lost!"));

        // 순찰(Patrol) 상태로 돌아감
        CurrentState = EEnemyState::Patrol;
    }
}

void AEnemyBase::ChaseOnSimpleFSM()
{
    // 타겟과의 거리계산
    float Distance = GetDistanceTo(TargetPlayer);

    //UE_LOG(LogTemp, Warning, TEXT("AI: Chasing Start, Distance: %.2f"), Distance);

    // 거리가 사거리보다 멀다면 다가가기
    if (Distance > AttackRange)
    {   
        // 방향 벡터
        FVector Direction = TargetPlayer->GetActorLocation() - GetActorLocation();

        // 하늘을 날아다니지 않는다면 위 방향 무시
        Direction.Z = 0.0f;

        // 이동방향 정규화
        Direction.Normalize();

        // 캐릭터에게 이동명령(플레이어가 입력하는 방식과 동일)
        AddMovementInput(Direction, 1.0f);
    }
    else
    {   
        // 사거리보다 가깝다면 공격 상태로 이동
        CurrentState = EEnemyState::Attack;
    }
}

void AEnemyBase::CallAttackEndOnSimpleFSM()
{   
    // 공격이 끝나면 추격상태로 이동
    // 공격이 중단되어도(Hit 등) 전환
    CurrentState = EEnemyState::Chase;
    bIsAttacking = false;
}

void AEnemyBase::ResetHitStateOnSimpleFSM()
{
    if (CurrentState == EEnemyState::Dead)
    {
        return;
    }

    // 다시 공격이 가능하도록 전환
    bIsAttacking = false;

    // 만약 타겟 플레이어가 있다면 추적, 없으면 일반(Patrol)
    CurrentState = (TargetPlayer) ? EEnemyState::Chase : EEnemyState::Patrol;
    UE_LOG(LogTemp, Warning, TEXT("AI: Hit Stun Ended, Returning to Normal"));
}
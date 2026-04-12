#include "EnemyBase.h"
#include "HealthComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"
#include "BlueprintGameplayTagLibrary.h"
#include "PlayerBase.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

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
        DetectionRange->SetSphereRadius(DetectionRadius);
    }

    // 상태 지정
    CurrentState = EEnemyState::Patrol;
}

void AEnemyBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!bUseSimpleFSM || bIsDead) return;

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
        if (!IsCharacterCanAction() || bIsAttacking)
        {
            break;
        }
        //UE_LOG(LogTemp, Warning, TEXT("AI: Attack"));
        // 이동 멈춤 및 공격 실행
        TryAttack();

        break;

    case EEnemyState::Hit:
        //UE_LOG(LogTemp, Warning, TEXT("AI: Hit"));
        // 넉백 중이므로 아무 행동도 하지 않고 대기 (GetHit 함수에서 타이머 실행됨)
        break;

    case EEnemyState::Dead:
        //UE_LOG(LogTemp, Warning, TEXT("AI: Dead"));
        // 이미 죽었으므로 아무것도 하지 않음

        break;
    }
}

void AEnemyBase::OnDeath_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("AI: Enemy OnDeath Called!"));

    // 부모공통로직 실행
    Super::OnDeath_Implementation();

    CurrentState = EEnemyState::Dead;
    
    PlayDeathAnimation();

    PrimaryActorTick.bCanEverTick = false;
}

void AEnemyBase::TryAttack() 
{
    if (bIsAttacking || !IsCharacterCanAction())
    {
        //UE_LOG(LogTemp, Warning, TEXT("C++: Attack Return"));
        return;
    }

    if (!TargetPlayer)
    {
        return;
    }

    // 캐릭터의 정면과 캐릭터로부터 적의 방향
    FVector DirectionToTarget = (TargetPlayer->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector ForwardVector = GetActorForwardVector();
    // 두 벡터의 내적을 통한 각도 계산, 1이면 정면, -1이면 뒤
    float DotResult = FVector::DotProduct(ForwardVector, DirectionToTarget);

    if (DotResult >= 0.0f)
    {   
        // 정면에 있다면 그냥 공격
        Attack();   
    }
    else
    {   
        // 뒤에 있으면 뒤돌아보고 공격
        DirectionToTarget.Z = 0.0f;
        SetActorRotation(DirectionToTarget.Rotation());
        Attack();
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

void AEnemyBase::StepForward()
{
    // 넉백중 등 이동 불가능 상태에선 무시
    if (!IsCharacterCanAction())
    {
        return;
    }

    // 캐릭터 무브먼트 컴포넌트 없으면 리턴
    if (!MovementComp)
    {
        return;
    }

    // 지면마찰력을 0으로 변경
    MovementComp->GroundFriction = 0.0f;

    // 속도 적용
    FVector ForwardDir = GetActorForwardVector();   // 캐릭터 전방 벡터 구하기
    FVector DashVelocity = ForwardDir * EnemyAttackStepForce;
    DashVelocity.Z = MovementComp->Velocity.Z;
    MovementComp->Velocity = DashVelocity;

    //UE_LOG(LogTemp, Warning, TEXT("AI: Change GroundFriction to 0 and Dash Start!, Velocity = %.2f"), DashVelocity.X);
}

void AEnemyBase::ResetAttackCooldown()
{
    bCanAttack = true;
    UE_LOG(LogTemp, Warning, TEXT("Enemy Attack Cooldown Ready!"));
}

bool AEnemyBase::GetHit(const FDamageData& DamageData)
{
    // 부모 로직 실행, 피격이 유효하지 않았다면 리턴
    if (!Super::GetHit(DamageData))
    {
        return false;
    }

    // 넉백 면역이 아니라면 공격 초기화 및 Hit 상태로 만듬
    if (!bIsKnockBackImmune)
    {   
        // 현재 상태를 Hit으로 변겅
        CurrentState = EEnemyState::Hit;

        // 현재 캐릭터의 물리적 속도를 제거 (이건 선택사항)
        GetCharacterMovement()->StopMovementImmediately();

        // Hit 플래그 활성화 및 공격 로직 초기화 (지면마찰력 초기화)
        bIsHit = true;
        EndAttackState();

        // 타이머 설정 및 타이머 이후 호출 함수 지정(ResetHitStateOnSimpleFSM) - Hit 상태 해제 타이머
        GetWorldTimerManager().ClearTimer(HitStunTimerHandle);
        GetWorldTimerManager().SetTimer(HitStunTimerHandle, this, &AEnemyBase::ResetHitStateOnSimpleFSM, StunDuration, false);
    }

    UPaperFlipbookComponent* MySprite = GetSprite();
    if (MySprite && HitMaterial)
    {
        // 되돌리기 위해 원래 머티리얼을 저장
        if (OriginalMaterial == nullptr)
        {
            OriginalMaterial = MySprite->GetMaterial(0);
        }

        // 0번 슬롯의 머티리얼을 덮어씌우기
        MySprite->SetMaterial(0, HitMaterial);
    }

    // 메테리얼 변경 타이머 호출
    GetWorldTimerManager().ClearTimer(HitFlashResetTimerHandle);
    GetWorldTimerManager().SetTimer(HitFlashResetTimerHandle, this, &AEnemyBase::ResetHitFlash, 0.1f, false);

    return true;
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
        if (bCanAttack)
        {
            // 사거리보다 가깝다면 공격 상태로 이동
            CurrentState = EEnemyState::Attack;
        }
        else
        {   
            // 플레이어를 처다봄
            FVector LookDirection = TargetPlayer->GetActorLocation() - GetActorLocation();
            LookDirection.Z = 0.0f;

            if (!LookDirection.IsNearlyZero())
            {
                SetActorRotation(LookDirection.Rotation());
            }
        }
        
    }
}

void AEnemyBase::CallAttackEndOnSimpleFSM()
{   
    // 공격이 끝나면 추격상태로 이동
    // 공격이 중단되어도(Hit 등) 전환
    CurrentState = EEnemyState::Chase;
    bIsAttacking = false;
    EndAttackState();

    // 쿨타임 돌임
    bCanAttack = false;

    // 랜덤 쿨타임 적용
    float RandomCooldown = FMath::RandRange(MinAttackCooldown, MaxAttackCooldown);

    // 타이머 적용하여 쿨타임 리셋시키기
    GetWorldTimerManager().ClearTimer(AttackCooldownTimerHandle);
    GetWorldTimerManager().SetTimer(AttackCooldownTimerHandle, this, &AEnemyBase::ResetAttackCooldown, RandomCooldown, false);

    UE_LOG(LogTemp, Warning, TEXT("Enemy Attack Finished! Attck Cooldown: %.2f"), RandomCooldown);
}

void AEnemyBase::ResetHitStateOnSimpleFSM()
{
    if (CurrentState == EEnemyState::Dead)
    {
        return;
    }

    // 플래그 초기화
    bIsHit = false;

    // 만약 타겟 플레이어가 있다면 추적, 없으면 일반(Patrol)
    CurrentState = (TargetPlayer) ? EEnemyState::Chase : EEnemyState::Patrol;
    UE_LOG(LogTemp, Warning, TEXT("AI: Hit Stun Ended, Returning to Normal"));
}

bool AEnemyBase::IsCharacterCanAction()
{
    bool bIsCanAct = Super::IsCharacterCanAction();

    // 행동 가능한 상태 또는 공격중이지 않은 상태
    bIsCanAct = bIsCanAct || !bIsHit;

    return bIsCanAct;
}

void AEnemyBase::ResetHitFlash()
{
    UPaperFlipbookComponent* MySprite = GetSprite();
    if (MySprite && OriginalMaterial)
    {
        // 다시 원래 머티리얼로 되돌리기
        MySprite->SetMaterial(0, OriginalMaterial);
    }
}
#include "DefaultCharBase.h"
#include "HealthComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "CustomDamageType.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PaperFlipbookComponent.h" 
#include "Materials/MaterialInstanceDynamic.h"
#include "NiagaraFunctionLibrary.h"

ADefaultCharBase::ADefaultCharBase()
{
    // 체력 컴포넌트 생성 및 부착
    HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    // 공격 판정 박스 생성 - RootComponent에 부착
    AttackBox = CreateDefaultSubobject<UBoxComponent>(TEXT("AttackBox"));
    AttackBox->SetupAttachment(RootComponent);
    AttackBox->SetCollisionProfileName(TEXT("Trigger"));
    AttackBox->SetGenerateOverlapEvents(true);
    AttackBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    // 아래 안되는거 같은니깐 그냥 블루프린트에서 ResponseToChannel 설정
    AttackBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
    AttackBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);

    AttackBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADefaultCharBase::BeginPlay()
{
    Super::BeginPlay();

    // 공격 박스 Overlap 이벤트 바인딩
    AttackBox->OnComponentBeginOverlap.AddDynamic(this, &ADefaultCharBase::OnAttackBoxOverlap);

    // 공격 박스 비활성화
    AttackBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // MovementComponent 할당
    MovementComp = GetCharacterMovement();

    // 플립북 컴포넌트 할당
    FlipbookComp = GetSprite();

    // 기존 바닥 마찰력 저장
    if (MovementComp)
    {
        SavedGroundFriction = MovementComp->GroundFriction;
    }

    // 캐릭터가 가진 플립북 컴포넌트를 가져옴
    UPaperFlipbookComponent* PaperSpriteComp = GetSprite();
    if (PaperSpriteComp)
    {
        // 0번 슬롯의 머티리얼을 기반으로 다이내믹 머티리얼 인스턴스 생성
        DynamicSpriteMat = PaperSpriteComp->CreateDynamicMaterialInstance(0);
    }
}

void ADefaultCharBase::ReceiveDamage_Implementation(const float DamageAmount)
{
    // HealthComponent가 유효하면 체력 감소 처리
    if (HealthComponent)
    {
        HealthComponent->ReduceHealth(DamageAmount);
    }

    // TODO: 피격 방향 넉백, 시선 전환 등
}

void ADefaultCharBase::OnDeath_Implementation()
{
    // 블루프린트에서 override해서 사망 애니메이션, 이펙트, 아이템 드롭 후 Destroy 
    //Destroy();
    UE_LOG(LogTemp, Warning, TEXT("C++: Character OnDeath"));

    if (!bIsDead)
    {
        bIsDead = true;
    }
}

float ADefaultCharBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{   
    // 사망상태라면 즉시 리턴
    if (bIsDead)
    {
        return 0;
    }

    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    
    // 인터페이스 전달을 위한 구조체 선언 및 초기화
    FDamageData Data;
    Data.DamageAmount = ActualDamage;
    Data.DamageCauser = DamageCauser;
    Data.HitDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();

    // 인자로 받은 데미지 타입 추출
    if (DamageEvent.DamageTypeClass)
    {   
        // 커스텀 데미지 타입으로 캐스팅
        UCustomDamageType* CustomDamage = Cast<UCustomDamageType>(DamageEvent.DamageTypeClass->GetDefaultObject());
        
        // 캐스팅된 커스텀 데미지 설정값을 DamageData로 복사
        if (CustomDamage)
        {
            Data.bIgnoreInvincible = CustomDamage->bIgnoreInvincible;
            Data.bIgnoreDodge = CustomDamage->bIgnoreDodge;
            Data.bIgnoreGuard = CustomDamage->bIgnoreGuard;
        }
    }

    // 피격 관리 함수 호출
    bool bIsHit = GetHit(Data);

    if (bIsHit)
    {
        return ActualDamage;   // 피해를 입힘
    }
    else
    {
        return 0.0f;   // 피해를 입히지 못함
    }
}

void ADefaultCharBase::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    // 내 주머니에 있는 모든 태그를 외부(TagContainer)로 복사해줌
    TagContainer = CharacterTags;
}

void ADefaultCharBase::TryAttack()
{
    // 상태검사, 공격 중 / 사망 / 넉백 / 피해무적상태(추후 추가 가능) 등등이라면 공격 불가
    // Enemy, Player클래스는 부모클래스를 사용하지 않고 재정의합니다. (함수 내부에서 호출하는 함수때문에 그럼)
    if (bIsAttacking || !IsCharacterCanAction())
    {
        //UE_LOG(LogTemp, Warning, TEXT("C++: Attack Return"));
        return;
    }

    Attack();
}

void ADefaultCharBase::Attack_Implementation()
{   
    // 공격 상태 진입, 기본 구현이라 자식에선 아마 실행 안할듯?
    //UE_LOG(LogTemp, Warning, TEXT("C++: Attack!(DefaultCharBase)"));
}

void ADefaultCharBase::StartAttackCollision()
{
    THitActors.Empty(); // 맞은 목록 초기화
    AttackBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ADefaultCharBase::EndAttackCollision()
{
    AttackBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADefaultCharBase::EndAttackState()
{
    bIsAttacking = false;

    // 지면마찰력 재적용
    if (MovementComp)
    {
        MovementComp->GroundFriction = SavedGroundFriction;
        //UE_LOG(LogTemp, Warning, TEXT("Default Char: Change GroundFriction to Normal"));
    }

    // 마찬가지로 BP에서 오버라이드 해 별도 필요 기능을 구현할 수 있음
}

bool ADefaultCharBase::CanAttackTarget(AActor* Target) const
{   
    // 타겟이 없거나 자기 자신이면 공격 불가
    if (!Target || Target == this) return false;

    // 기본적으로 true 반환 (자식 클래스에서 구체화)
    return true;
}

void ADefaultCharBase::OnAttackBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{   
    // OtherActor가 없거나, 본인이거나, 중복피격배열에 OtherActor가 있다면 미실행
    if (OtherActor && OtherActor != this && !THitActors.Contains(OtherActor))
    {
        if (CanAttackTarget(OtherActor))
        {
            THitActors.Add(OtherActor);

            ExecuteAttackHit(OtherActor, CurrentAttackDamageType);
        }
    }

}

void ADefaultCharBase::ExecuteAttackHit(AActor* TargetActor, TSubclassOf<class UCustomDamageType> DamageType, float DamageMultiplier)
{
    if (!TargetActor)
    {
        return;
    }

    // 인자로 받은 데미지타입 지정
    TSubclassOf<UCustomDamageType> DamageTypeToUse = DamageType;

    // 디테일 패널에서 데미지 타입을 정하지 않았거나 별도의 데미지 타입 지정이 없다면 기본 데미지 타입 적용(무적,회피,가드 영향을 받는 상태)
    if (!DamageTypeToUse)
    {
        DamageTypeToUse = UCustomDamageType::StaticClass();
    }

    // 피해량 배수 지정
    float FinalDamage = DefaultDamage * DamageMultiplier;

    // 적에게 피해 적용
    float ActualDamage = UGameplayStatics::ApplyDamage(TargetActor, FinalDamage, GetController(), this, DamageTypeToUse);

    if (ActualDamage > 0.0f)
    {
        //UE_LOG(LogTemp, Warning, TEXT("DefaultCharBase: Apply Hit Stop Custom"));
        // 자신에게 히트스탑 적용
        ApplyHitStopCustom(0.1f, 0.1f);
    }

    
}

void ADefaultCharBase::SetDefaultDamage(float Amount)
{
    DefaultDamage = Amount;
}

void ADefaultCharBase::ApplyHitStopGlobal(float Duration, float Dilation)
{
    // 인자로 주어진 시간만큼으로 월드 타임 감속
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), Dilation);

    FTimerHandle HitStopTimerHandle;

    GetWorldTimerManager().SetTimer(HitStopTimerHandle, FTimerDelegate::CreateLambda([this]()
        {
            // 0.025초 뒤에 다시 원래 속도로 복귀
            UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

        }), Duration, false);
}

void ADefaultCharBase::ApplyHitStopCustom(float Duration, float Dilation)
{
    // 인자로 주어진 시간만큼 커스텀 시간 감속
    this->CustomTimeDilation = Dilation;

    FTimerHandle HitStopTimerHandle;

    GetWorldTimerManager().SetTimer(HitStopTimerHandle, FTimerDelegate::CreateLambda([this]()
        {
            // 0.025초 뒤에 다시 원래 속도로 복귀
            this->CustomTimeDilation = 1.0f;

        }), Duration, false);
}

bool ADefaultCharBase::GetHit(const FDamageData& DamageData)
{   
    // 1. 무적상태일때의 판정
    if (bIsInvincible)
    {   
        UE_LOG(LogTemp, Warning, TEXT("C++ DefaultCharBase: Is Invincible!"));

        // 무적상태이지만, 공격이 무적을 무시할 경우(낙하데미지와 같은 환경데미지) 즉시 데미지 적용 및 리턴
        if (DamageData.bIgnoreInvincible)
        {
            // IDamageable::ReceiveDamage로 연결 후 데이터 전달
            if (this->Implements<UDamageable>())
            {   
                UE_LOG(LogTemp, Warning, TEXT("C++ DefaultCharBase: Invincible ignored!"));
                IDamageable::Execute_ReceiveDamage(this, DamageData.DamageAmount);
            }
            return true;
        }

        // 무적상태일경우 리턴
        return false;
    }

    // 2. 회피중일때의 판정
    if (bIsDodging)
    {   
        // 받은 데미지가 회피 불가능한 속성(장판, 기믹공격)이라면, 회피를 무시하고 피해를 입힘
        if (DamageData.bIgnoreDodge)
        {
            UE_LOG(LogTemp, Warning, TEXT("C++ DefaultCharBase: Dodge ignored!"));
        }
        // 회피 가능 공격의 경우
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("C++ DefaultCharBase: Dodge success!"));
            return false;
        }
    }

    // 3. 가드 판정
    if (bIsGuarding)
    {   
        if (DamageData.bIgnoreGuard)
        {
            UE_LOG(LogTemp, Warning, TEXT("C++ DefaultCharBase: Guard ignored!"));
        }
        else
        {
            // 캐릭터가 바라보는 전방 벡터
            FVector Forward = GetActorForwardVector();

            // 캐릭터부터 공격자를 향하는 방향벡터, 높이차이는 무시
            FVector DirToAttacker = (DamageData.DamageCauser->GetActorLocation() - GetActorLocation());
            DirToAttacker.Z = 0.0f;
            DirToAttacker = DirToAttacker.GetSafeNormal();

            // 두 벡터의 내적 계산, 두 벡터의 내적이 0보다 크면 전방에 공격자가 있다는 의미
            float DotResult = FVector::DotProduct(Forward, DirToAttacker);

            // 가드 판정 성공
            if (DotResult > 0.0f)
            {
                UE_LOG(LogTemp, Warning, TEXT("C++: Guard Success!"));

                // 가드 성공 신호
                bIsGuardSuccess = true;

                // 가드 성공 이펙트 실행
                if (GuardEffect)
                {
                    // 이펙트가 향할 방향은 피격받은 캐릭터로부터 피격당한 방향
                    FVector GuardEffectDirection = DamageData.HitDirection * -1.0f;
                    FRotator GuardEffectRotation = GuardEffectDirection.Rotation();

                    // 나이아가라 이펙트가 스폰 될 위치(해당 캐릭터의 위치)
                    FVector GuardEffectSpawnLocation = GetActorLocation();
                    GuardEffectSpawnLocation.Y += 50.0f;
                    GuardEffectSpawnLocation.Z += 30.0f;

                    // 이펙트 스폰
                    UNiagaraFunctionLibrary::SpawnSystemAtLocation(
                        GetWorld(),
                        GuardEffect,
                        GuardEffectSpawnLocation,
                        GuardEffectRotation
                    );
                }

                // 가드넉백만큼의 넉백 적용
                if (MovementComp)
                {
                    // 지면마찰력을 0으로 변경
                    MovementComp->GroundFriction = 0.0f;
                    FVector DashVelocity = Forward * GuardKnockbackStrength * -1;
                    DashVelocity.Z = MovementComp->Velocity.Z;
                    MovementComp->Velocity = DashVelocity;
                }

                // 가드가 만약 한번만의 공격을 막는다면
                if (bIsGuardBlockOnlyOneHit)
                {
                    UE_LOG(LogTemp, Warning, TEXT("C++: Guard Apply One Time!"));

                    // 가드 타이머 해제
                    GetWorldTimerManager().ClearTimer(GuardTimerHandle);

                    // 가드상태 즉시해제
                    EndGuard();
                }

                // 0.2초 이후에 가드상태(지면 마찰력) 원래상태로 초기화
                GetWorldTimerManager().SetTimer(GuardRecoilTimerHandle, this, &ADefaultCharBase::RestoreGuardState, 0.2f, false);

                return false;
            }
        }
    }

    // 피격 상태 진입
    UE_LOG(LogTemp, Warning, TEXT("C++ DefaultCharBase: Get Hit!"));

    // 나이아가라 피격 이펙트 스폰
    if (HitEffect)
    {   
        // 이펙트가 향할 방향은 피격받은 캐릭터로부터 피격당한 방향
        FVector HitEffectDirection = DamageData.HitDirection * -1.0f;
        FRotator HitEffectRotation = HitEffectDirection.Rotation();

        // 나이아가라 이펙트가 스폰 될 위치(해당 캐릭터의 위치)
        FVector HitEffectSpawnLocation = GetActorLocation();
        HitEffectSpawnLocation.Y += 50.0f;

        // 이펙트 스폰
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            HitEffect,
            HitEffectSpawnLocation,
            HitEffectRotation
        );
    }

    // 넉백에 면역이 아닌 경우에
    if (!bIsKnockBackImmune)
    {
        // 넉백 실행
        PlayKnockBack(DamageData);

        // 피격 애니메이션 재생
        PlayHitAnimation();
    }

    // IDamageable::ReceiveDamage로 연결 후 데이터 전달 (실제 피해 적용)
    if (this->Implements<UDamageable>())
    {
        IDamageable::Execute_ReceiveDamage(this, DamageData.DamageAmount);
    }

    // 피격되었으므로, true 리턴
    return true;
}

void ADefaultCharBase::PlayKnockBack(const FDamageData& DamageData)
{
    FVector LaunchVelocity = DamageData.HitDirection * KnockbackStrength; // 넉백 강도
    LaunchVelocity.Z = KnockbackZStregth; // 살짝 위로 뜨게 만듦

    LaunchCharacter(LaunchVelocity, true, true); // 넉백 적용
}

bool ADefaultCharBase::TryDodge(float Time)
{   
    // 만약 회피가 불가능한 상황이면 회피하지 않고 false 리턴
    if (!IsCharacterCanAction() || bIsAttacking || !bCanDodge)
    {   
        return false;
    }
    
    // 회피 가능하면 회피함수 호출
    DodgeStart(Time);
    return true;
}

void ADefaultCharBase::DodgeStart(float Time)
{
    // 쿨타임 적용
    bCanDodge = false;

    // 회피상태 활성화
    bIsDodging = true;

    // 회피 애니메이션 재생
    //PlayDodgeAnimation();

    // 가드 성공 후의 넉백 타이머를 초기화
    GetWorldTimerManager().ClearTimer(GuardRecoilTimerHandle);

    // 기존 회피 타이머가 있다면 강제 초기화
    GetWorldTimerManager().ClearTimer(DodgeTimerHandle);

    // DodgeDuration 이후에 회피 종료 타이머 실행
    GetWorldTimerManager().SetTimer(DodgeTimerHandle, this, &ADefaultCharBase::DodgeEnd, Time, false);

    //UE_LOG(LogTemp, Warning, TEXT("C++ DefaultCharBase: Now Dodge Start!, Duration: %.2f"), Time);

    // 쿨타임 적용 타이머
    GetWorldTimerManager().SetTimer(
        DodgeCooldownTimerHandle,
        this,
        &ADefaultCharBase::ResetDodgeCooldown,
        DodgeCooldown,
        false
    );
}

void ADefaultCharBase::DodgeEnd()
{   
    // 회피상태 종료
    bIsDodging = false;

    if (MovementComp)
    {
        MovementComp->GroundFriction = SavedGroundFriction;
    }

    // UE_LOG(LogTemp, Warning, TEXT("C++ DefaultCharBase: Now Dodge End!"));
}

void ADefaultCharBase::ResetDodgeCooldown()
{
    bCanDodge = true;
    // UE_LOG(LogTemp, Warning, TEXT("Dodge is Ready again!"));
}

void ADefaultCharBase::ApplySpriteSortAmount()
{
    if (FlipbookComp)   
    {   
        // 현재 스프라이트의 로컬 위치 가져오기
        CurrentRelativeLoc = FlipbookComp->GetRelativeLocation();

        if (GetActorForwardVector().X >= 0)
        {
            // 우측(앞)을 볼 때는 기본 오프셋
            CurrentRelativeLoc.Y = SpriteLayerSortAmount;
        }
        else
        {
            // 좌측(뒤)을 볼 때는 로컬 Y축도 180도 돌아갔으므로 마이너스로 상쇄
            CurrentRelativeLoc.Y = -SpriteLayerSortAmount;
        }

        // 상대위치 적용 
        FlipbookComp->SetRelativeLocation(CurrentRelativeLoc);
    }
}

bool ADefaultCharBase::TryGuard()
{   
    // 행동 불가 상태에선 가드 불가능
    if (!IsCharacterCanAction())
    {
        return false;
    }

    if (bIsAttacking)
    {
        // TODO: 공격 도중 가드 허용 여부 및 로직 구현
        return false;
    }

    GuardStart();
    return true;
}

void ADefaultCharBase::GuardStart()
{   
    // 가드 플래그 설정
    bCanGuard = false;
    bIsGuarding = true;

    // 자식 PlayerBase - 방향키 방향으로 전환 및 이동 즉시 중단
    // 자식 EnemyBase - 딱히 없음, BP에서 별도 구현

    // 가드 애니메이션 재생
    PlayGuardAnimation();

    // 가드 타이머 초기화
    GetWorldTimerManager().ClearTimer(GuardTimerHandle);
    
    // GuardDuration동안 가드 타이머 가동
    GetWorldTimerManager().SetTimer(GuardTimerHandle, this, &ADefaultCharBase::EndGuard, GuardDuration, false);

    // 가드 쿨타임 타이머 가동
    GetWorldTimerManager().SetTimer(GuardCooldownTimerHandle, this, &ADefaultCharBase::ResetGuardCooldown, GuardCoolDown, false);

    UE_LOG(LogTemp, Warning, TEXT("C++ DefaultCharBase: Guard Started!"));
}

void ADefaultCharBase::EndGuard()
{   
    // 자식 BP에서 구체화 필요
    if (!bIsGuarding)
    {
        return;
    }

    // 지면마찰력 복구는 안하나요? GetHit 함수에서 지면마찰력 복구 함수가 있습니다. 가드시 뒤로 밀려나는 효과를 위해 타이머로 조작합니다. 

    // 가드상태 해제 
    bIsGuarding = false;

    //// 가드 성공 플래그 초기화
    //if (bIsGuardSuccess)
    //{
    //    bIsGuardSuccess = false;
    //}
    
    UE_LOG(LogTemp, Warning, TEXT("C++ DefaultCharBase: Guard Ended!"));
}

void ADefaultCharBase::ResetGuardCooldown()
{
    bCanGuard = true;
}

void ADefaultCharBase::RestoreGuardState()
{   
    // 가드 성공 플래그 초기화
    if (bIsGuardSuccess)
    {
        bIsGuardSuccess = false;
    }

    if (MovementComp)
    {
        MovementComp->GroundFriction = SavedGroundFriction;
    }
}

bool ADefaultCharBase::IsCharacterCanAction()
{
    bool bIsCanAct = !(bIsKnockBack || bIsDead || bIsDodging || bIsGuarding);

    return bIsCanAct;
}

void ADefaultCharBase::ResetCombatStates()
{
    // 1. 회피 관련 플래그 및 자원 카운트 강제 초기화
    bIsDodging = false;
    bCanDodge = true;
    GetWorldTimerManager().ClearTimer(DodgeTimerHandle);
    GetWorldTimerManager().ClearTimer(DodgeCooldownTimerHandle);

    // 2. 이동/점프 관련 플래그 및 자원 카운트 초기화
    MovementComp->GroundFriction = SavedGroundFriction;

    // 3. 가드 관련 플래그 초기화
    bIsGuarding = false;
    bIsGuardSuccess = false;
    bCanGuard = true;
    GetWorldTimerManager().ClearTimer(GuardTimerHandle);
    GetWorldTimerManager().ClearTimer(GuardCooldownTimerHandle);
    GetWorldTimerManager().ClearTimer(GuardRecoilTimerHandle);
}
#include "DefaultCharBase.h"
#include "HealthComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"


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
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    
    // 인터페이스 전달을 위한 구조체 선언 및 초기화
    FDamageData Data;
    Data.DamageAmount = ActualDamage;
    Data.DamageCauser = DamageCauser;
    Data.HitDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();

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
    // Enemy 클래스는 그대로 사용 가능, Player클래스는 부모클래스를 사용하지 않고 재정의합니다. 
    if (bIsAttacking || bIsKnockBack)
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

            // 적에게 피해 적용
            float ActualDamage = UGameplayStatics::ApplyDamage(OtherActor, DefaultDamage, GetController(), this, UDamageType::StaticClass());

            if (ActualDamage > 0.0f)
            {   
                UE_LOG(LogTemp, Warning, TEXT("C++ DefaultCharBase: Apply Hit Stop Custom"));
                // 자신에게 히트스탑 적용
                ApplyHitStopCustom(0.05f, 0.01f);
            }
        }
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

    // 피격 상태 진입
    UE_LOG(LogTemp, Warning, TEXT("C++ DefaultCharBase: Get Hit!"));

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

    return true;
}

void ADefaultCharBase::PlayKnockBack(const FDamageData& DamageData)
{
    FVector LaunchVelocity = DamageData.HitDirection * KnockbackStrength; // 넉백 강도
    LaunchVelocity.Z = 200.f; // 살짝 위로 뜨게 만듦

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
    PlayDodgeAnimation();

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

    UE_LOG(LogTemp, Warning, TEXT("C++ DefaultCharBase: Now Dodge End!"));
}

void ADefaultCharBase::ResetDodgeCooldown()
{
    bCanDodge = true;
    UE_LOG(LogTemp, Warning, TEXT("Dodge is Ready again!"));
}

bool ADefaultCharBase::IsCharacterCanAction()
{
    bool bIsCanAct = !(bIsKnockBack || bIsDead || bIsDodging);

    return bIsCanAct;
}
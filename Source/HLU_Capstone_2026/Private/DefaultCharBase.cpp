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
}

void ADefaultCharBase::BeginPlay()
{
    Super::BeginPlay();

    // 공격 박스 Overlap 이벤트 바인딩
    AttackBox->OnComponentBeginOverlap.AddDynamic(this, &ADefaultCharBase::OnAttackBoxOverlap);

    // 델리게이트 바인딩(HealthComponent의 FOnTakeDamageSignature와 바인딩 하여 피격 정보를 받음)
    if (HealthComponent)
    {
        HealthComponent->OnTakeDamage.AddDynamic(this, &ADefaultCharBase::PlayKnockBack);
    }
}

void ADefaultCharBase::ReceiveDamage_Implementation(const FDamageData& DamageData)
{
    // HealthComponent가 유효하면 체력 감소 처리
    if (HealthComponent)
    {
        HealthComponent->ReduceHealth(DamageData);
    }

    // TODO: 피격 방향 넉백, 시선 전환 등
}

void ADefaultCharBase::OnDeath_Implementation()
{
    // 블루프린트에서 override해서 사망 애니메이션, 이펙트, 아이템 드롭 후 Destroy 
    //Destroy();
    UE_LOG(LogTemp, Warning, TEXT("Character OnDeath"));
}

float ADefaultCharBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    // 인터페이스 전달을 위한 구조체 선언 및 초기화
    FDamageData Data;
    Data.DamageAmount = ActualDamage;
    Data.DamageCauser = DamageCauser;
    Data.HitDirection = (GetActorLocation() - DamageCauser->GetActorLocation()).GetSafeNormal();

    // IDamageable::ReceiveDamage로 연결 후 데이터 전달
    if (this->Implements<UDamageable>())
    {
        IDamageable::Execute_ReceiveDamage(this, Data);
    }

    return ActualDamage;
}

void ADefaultCharBase::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
    // 내 주머니에 있는 모든 태그를 외부(TagContainer)로 복사해줌
    TagContainer = CharacterTags;
}

void ADefaultCharBase::Attack_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("DefaultCharBase: Attack!"));
    // 블루프린트에서 override해서 구현
    // 공격 애니메이션 재생, AttackBox 활성화 타이밍 조정 등
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

bool ADefaultCharBase::CanAttackTarget(AActor* Target) const
{   
    // 타겟이 없거나 자기 자신이면 공격 불가
    if (!Target || Target == this) return false;

    // 기본적으로 true 반환 (자식 클래스에서 구체화)
    return true;
}

void ADefaultCharBase::OnAttackBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{   
    // OtherActor가 없거나, 본이이거나, 중복피격배열에 OtherActor가 있다면 미실행
    if (OtherActor && OtherActor != this && !THitActors.Contains(OtherActor))
    {
        if (CanAttackTarget(OtherActor))
        {
            THitActors.Add(OtherActor);

            // 적에게 피해 적용
            UGameplayStatics::ApplyDamage(OtherActor, DefaultDamage, GetController(), this, UDamageType::StaticClass());
        }
    }

}

void ADefaultCharBase::SetDefaultDamage(float Amount)
{
    DefaultDamage = Amount;
}

void ADefaultCharBase::PlayKnockBack(const FDamageData& DamageData)
{
    // 넉백당하지 않는 상태라면(bIsKnockBack == false) 리턴
    if (bIsKnockBack == false)
    {
        return;
    }

    FVector LaunchVelocity = DamageData.HitDirection * KnockbackStrength; // 넉백 강도
    LaunchVelocity.Z = 200.f; // 살짝 위로 뜨게 만듦

    LaunchCharacter(LaunchVelocity, true, true); // 넉백 적용
}
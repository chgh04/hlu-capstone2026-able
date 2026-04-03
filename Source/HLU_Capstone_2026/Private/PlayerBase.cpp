#include "PlayerBase.h"
#include "BlueprintGameplayTagLibrary.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

APlayerBase::APlayerBase()
{
    // 카메라 암 생성 및 설정
    CameraString = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraString->SetupAttachment(RootComponent); // 캐릭터의 루트 부착
    CameraString->TargetArmLength = 400.0f;       // 캐릭터와의 거리 설정(디테일창 변경 가능)
    CameraString->bUsePawnControlRotation = true; // 컨트롤러 회전에 따라 암도 회전

    // 카메라 생성
    MainCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    MainCamera->SetupAttachment(CameraString, USpringArmComponent::SocketName);
    MainCamera->bUsePawnControlRotation = false;
}

void APlayerBase::BeginPlay()
{
    Super::BeginPlay();
}

void APlayerBase::TryAttack()
{   
    // 부모클래스 TryAttack을 실행하지 않고 완전히 재정의
    // 1. 상태검사, 사망 / 넉백 / 피해무적상태(추후 추가 가능) 등등이라면 공격 불가
    if (bIsKnockBack)
    {
        UE_LOG(LogTemp, Warning, TEXT("C++ Player Attack Return: Other Reason"));
        return;
    }

    // 2. 플레이어가 공격 애니메이션 도중 입력이 되었을 때, 공격 예약
    if (bIsAttacking)
    {   
        if (!bSaveAttack && !bIgnoreSaveAttack)
        {
            bSaveAttack = true;
            UE_LOG(LogTemp, Warning, TEXT("C++ Player Attack Return: Attack saved"));
        }
        //UE_LOG(LogTemp, Warning, TEXT("C++ Player Attack Return: Already attacking"));
        return;
    }

    // 3. 공격 애니메이션은 끝났지만, 대기시간 내에 입력을 했을 때
    if (bIsWaitNextAttackInput)
    {   
        GetWorldTimerManager().ClearTimer(ComboTimerHandle);
        bIsWaitNextAttackInput = false;
        ComboCount++;
        UE_LOG(LogTemp, Warning, TEXT("C++ Attack Continued! ComboCount: %d"), ComboCount);
        Attack();
        return;
    }

    // 공격 초기화 타이머의 Timer ghost 현상 방지
    GetWorldTimerManager().ClearTimer(ComboTimerHandle);

    // 4. 첫 번째 공격일 경우
    ComboCount = 0;
    UE_LOG(LogTemp, Warning, TEXT("C++ First Attack Started"));
    Attack();
}

void APlayerBase::Attack_Implementation()
{   
    // 부모 Attack 함수 미실행
    //Super::Attack_Implementation();

    // 공격이 시작될 때 모든 플래그를 초기화
    bIsAttacking = true;
    bSaveAttack = false;
    bIsWaitNextAttackInput = false;

    //UE_LOG(LogTemp, Warning, TEXT("C++: Now Attack!(PlayerBase)"));
}

bool APlayerBase::CanAttackTarget(AActor* Target) const
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

    FGameplayTag EnemyTag = FGameplayTag::RequestGameplayTag(FName("Entity.Team.Enemy"));
    FGameplayTag DestructibleTag = FGameplayTag::RequestGameplayTag(FName("Entity.Type.Destructible"));

    // 타겟이 "Enemy" 태그를 가지고 있거나, "Destructible(파괴가능)" 태그가 있다면 공격 가능
    return TagInterface->HasMatchingGameplayTag(EnemyTag) || TagInterface->HasMatchingGameplayTag(DestructibleTag);
}

void APlayerBase::ApplyHitStop(float time)
{   
    // 인자로 주어진 시간만큼으로 월드 타임 감속
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), time);

    FTimerHandle HitStopTimerHandle;

    GetWorldTimerManager().SetTimer(HitStopTimerHandle, FTimerDelegate::CreateLambda([this]()
        {
            // 0.05초 뒤에 다시 원래 속도로 복귀
            UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

        }), 0.05f, false);
}

void APlayerBase::PlayerAttackDelay(float Time)
{   
    // 기존 공격 딜레이 타이머 제거
    GetWorldTimerManager().ClearTimer(PlayerTimerHandle);

    // Time동안 공격 딜레이 상태 전환
    GetWorldTimerManager().SetTimer(PlayerTimerHandle, this, &APlayerBase::PlayerAttackDelayReset, Time, false);
}

void APlayerBase::PlayerAttackDelayReset()
{
    
}

void APlayerBase::CheckCombo()
{   
    // 공격 연계가 가능하도록 전환
    bIgnoreSaveAttack = false;
}

void APlayerBase::ResetCombo()
{   
    // 중복호출 제한
    if (!bIsAttacking)
    {
        return;
    }

    // UE_LOG(LogTemp, Warning, TEXT("Reset Combo Called!"));

    // 선입력 되었다면 즉시 다음 공격 실행
    if (bSaveAttack)
    {   
        ComboCount++;
        UE_LOG(LogTemp, Warning, TEXT("C++ Auto Attack Triggered! ComboCount: %d"), ComboCount);
        Attack();
        return;
    }
    
    // 콤보카운트가 0일때(첫 번째 공격)만 2번째 공격으로 연계가 가능
    if (ComboCount % 2 == 0)
    {
        bIsWaitNextAttackInput = true;
    }
    
    // 공격상태 초기화 / 공격 연계 가능 구간 플래그 초기화
    EndAttackState();

    // SecondAttackWaitTime 이후에 모든 콤보 플래그 리셋 함수를 호출
    UE_LOG(LogTemp, Warning, TEXT("C++ Player Wait Next Input, WaitTime: %.2f"), SecondAttackWaitTime);
    GetWorldTimerManager().SetTimer(ComboTimerHandle, this, &APlayerBase::FullResetCombo, SecondAttackWaitTime, false);
}

void APlayerBase::EndAttackState()
{   
    // 부모로직(bIsAttacking = false) 호출
    Super::EndAttackState();

    // 공격 연계 가능 구간 플래그 초기화
    bIgnoreSaveAttack = true;
}

void APlayerBase::FullResetCombo()
{   
    // 공격 도중이면 리셋 방지
    if (bIsAttacking)
    {
        UE_LOG(LogTemp, Warning, TEXT("C++: FullResetCombo Ignored! Player is already attacking."));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("C++: Full reset combo"));
    bIsAttacking = false;
    bSaveAttack = false;
    bIsWaitNextAttackInput = false;
    ComboCount = 0;

    // UE_LOG(LogTemp, Warning, TEXT("C++: Combo Reset(PlayerBase)"));
}

void APlayerBase::GetHit(const FDamageData& DamageData)
{   
    // 부모클래스 GetHit 먼저 실행 (넉백 면역이 아닌 경우에 넉백 적용 및 HitAnimation 강제 재생(BP에서 정의됨))
    Super::GetHit(DamageData);
    
    // 0.1초간 히트스탑
    ApplyHitStop(0.1f);

    // 콤보 관련 타이머 진행상황을 즉시 강제 종료
    GetWorldTimerManager().ClearTimer(ComboTimerHandle);

    // 플레이어 넉백상태 전환(공격 및 이동 불가능) 및 무적 적용
    bIsKnockBack = true;
    bIsInvincible = true;

    // 모든 공격 플래그 강제 리셋
    EndAttackState();   // 공격 상태 false로 전환 및 연계 가능 구간 플래그 초기화
    bSaveAttack = false;    // 선입력된 공격 중단
    bIsWaitNextAttackInput = false; // 대기시간 중 입력된 공격 중단 
    ComboCount = 0; // 콤보 초기화

    // 기존에 돌고 있던 피격 타이머가 있다면 초기화 (연속 피격 대비)
    GetWorldTimerManager().ClearTimer(HitInvincibleTimerHandle);

    // 주어진 시간 후에 무적상태/넉백상태 해제 및 공격상태 해제(공격 중 피격 시 플래그가 바뀌지 않는 문제 해결)
    GetWorld()->GetTimerManager().SetTimer(HitInvincibleTimerHandle, FTimerDelegate::CreateLambda([this]()
        {
            bIsInvincible = false;
            bIsKnockBack = false;

            UE_LOG(LogTemp, Warning, TEXT("C++: Hit Stun & Invincible Ended"));
        }), HitInvincibleTime, false);
}

void APlayerBase::TryJump()
{   
    // 점프가 불가능한 상황에서는 점프 불가
    if (bIsKnockBack)
    {
        return;
    }

    Jump();
}

void APlayerBase::TryStopJumping()
{   
    // 점프 중단, 추후 별도 로직 추가 가능
    StopJumping();
}
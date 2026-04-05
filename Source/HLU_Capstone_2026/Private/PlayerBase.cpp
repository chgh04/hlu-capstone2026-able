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

    MovementComp = GetCharacterMovement();
    
    // 기존 바닥 마찰력 저장
    if (MovementComp)
    {
        SavedGroundFriction = MovementComp->GroundFriction;
    }
}

void APlayerBase::TryAttack()
{   
    // 부모클래스 TryAttack을 실행하지 않고 완전히 재정의
    // 1. 상태검사, 사망 / 넉백 / 피해무적상태(추후 추가 가능) 등등이라면 공격 불가
    if (!IsCharacterCanAction())
    {
        //UE_LOG(LogTemp, Warning, TEXT("C++ Player Attack Return: Other Reason"));
        return;
    }

    // 2. 플레이어가 공격 애니메이션 도중 입력이 되었을 때, 공격 예약
    if (bIsAttacking)
    {   
        if (!bSaveAttack && !bIgnoreSaveAttack)
        {
            bSaveAttack = true;
            //UE_LOG(LogTemp, Warning, TEXT("C++ Player Attack Return: Attack saved"));
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
        //UE_LOG(LogTemp, Warning, TEXT("C++ Attack Continued! ComboCount: %d"), ComboCount);
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

    // 공격 브레이크 전 속도 저장
    SavedAttackSpeed = GetVelocity().Size2D();

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

void APlayerBase::ApplyHitStop(float time, float dilation)
{   
    // 인자로 주어진 시간만큼으로 월드 타임 감속
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), dilation);

    FTimerHandle HitStopTimerHandle;

    GetWorldTimerManager().SetTimer(HitStopTimerHandle, FTimerDelegate::CreateLambda([this]()
        {
            // 0.025초 뒤에 다시 원래 속도로 복귀
            UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
            
        }), time, false);
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

    // 회피 선입력이 있는지 판단, 회피 선입력이 눌려질 시 선입력된 공격은 취소
    if (bSaveDodge && bCanDodge)
    {   
        UE_LOG(LogTemp, Warning, TEXT("Dodge Input Bufferd After Attack!"));
        bSaveDodge = false;
        bSaveAttack = false;

        // 즉시 공격상태 초기화
        EndAttackState();

        // 모든 콤보 변수 초기화
        FullResetCombo();

        // 회피 시작
        DodgeStart(DodgeDuration);
        return;
    }

    // 선입력 되었다면 즉시 다음 공격 실행
    if (bSaveAttack)
    {   
        ComboCount++;
        //UE_LOG(LogTemp, Warning, TEXT("C++ Auto Attack Triggered! ComboCount: %d"), ComboCount);
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

    UE_LOG(LogTemp, Warning, TEXT("Combo State End"));

    // SecondAttackWaitTime 이후에 모든 콤보 플래그 리셋 함수를 호출
    //UE_LOG(LogTemp, Warning, TEXT("C++ Player Wait Next Input, WaitTime: %.2f"), SecondAttackWaitTime);
    GetWorldTimerManager().SetTimer(ComboTimerHandle, this, &APlayerBase::FullResetCombo, SecondAttackWaitTime, false);
}

void APlayerBase::EndAttackState()
{   
    // 지면마찰력 재적용
    if (MovementComp)
    {
        MovementComp->GroundFriction = SavedGroundFriction;
    }

    // 부모로직(bIsAttacking = false) 호출
    Super::EndAttackState();

    // 공격 연계 가능 구간 플래그 초기화
    bIgnoreSaveAttack = true;
}

bool APlayerBase::IdleAttackWaitTrasitionFlag()
{
    return bIsWaitNextAttackInput || bIsAttacking;
}

void APlayerBase::FullResetCombo()
{   
    // 공격 도중이면 리셋 방지
    if (bIsAttacking)
    {
        //UE_LOG(LogTemp, Warning, TEXT("C++: FullResetCombo Ignored! Player is already attacking."));
        return;
    }

    //UE_LOG(LogTemp, Warning, TEXT("C++: Full reset combo"));
    bIsAttacking = false;
    bSaveAttack = false;
    bIsWaitNextAttackInput = false;
    ComboCount = 0;

    // UE_LOG(LogTemp, Warning, TEXT("C++: Combo Reset(PlayerBase)"));
}

bool APlayerBase::GetHit(const FDamageData& DamageData)
{   
    // 부모 로직 실행, 피격이 유효하지 않았다면 리턴
    if (!Super::GetHit(DamageData))
    {
        return false;
    }
    
    // 0.05초간 히트스탑
    ApplyHitStop(HitStopTime, HitStopDilation);

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

    return true;
}

void APlayerBase::TryJump()
{   
    // 점프가 불가능한 상황에서는 점프 불가
    if (!IsCharacterCanAction() || bIsAttacking)
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

void APlayerBase::StepForward(float StepForce)
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

    // 달리는 도중의 판단값 설정
    float RunThreshold = MovementComp->MaxWalkSpeed * 0.8f;

    // 배율 결정
    float SpeedMultiplier = (SavedAttackSpeed > RunThreshold) ? AttackStepForceMultiplierWhileRun : 1.0f;

    // 최종 힘 계산
    float FinalForce = StepForce * SpeedMultiplier;

    // 지면마찰력을 0으로 변경
    MovementComp->GroundFriction = 0.0f;

    // 속도 적용
    FVector ForwardDir = GetActorForwardVector();   // 캐릭터 전방 벡터 구하기
    FVector DashVelocity = ForwardDir * FinalForce;
    DashVelocity.Z = MovementComp->Velocity.Z;

    MovementComp->Velocity = DashVelocity;

    //UE_LOG(LogTemp, Warning, TEXT("Current Speed: %f, Final Force: %f, Threshold: %f"), SavedAttackSpeed, FinalForce, RunThreshold);
}

bool APlayerBase::TryDodge(float Time)
{
    // 부모클래스 함수를 호출하지 않습니다. 
    /*if (!Super::TryDodge(Time))
    {
        return false;
    }*/

    UE_LOG(LogTemp, Warning, TEXT("Dodge Tried!"));

    // 만약 회피가 불가능한 상황이면 회피하지 않고 false 리턴
    if (!IsCharacterCanAction())
    {   
        UE_LOG(LogTemp, Warning, TEXT("Dodge Ignored! Character Cannot Action!"));
        return false;
    }

    // 공격 도중 선입력이 있었거나 회피 불가능한 상황에서 입력이 있었을 경우
    if (bIsAttacking || !bCanDodge)
    {   
        // 회피 선입력 트리거를 활성화
        bSaveDodge = true;
        UE_LOG(LogTemp, Warning, TEXT("Dodge Input Buffered"));

        // 0.n초 뒤 예약 자동 해제
        FTimerHandle BufferTimer;
        GetWorldTimerManager().SetTimer(BufferTimer, FTimerDelegate::CreateLambda([this]() {
                bSaveDodge = false;
                UE_LOG(LogTemp, Warning, TEXT("Dodge Input Buffer Canceled"));
            }), 0.3f, false);

        // 회피가 실행되지 않았으니 false 리턴
        return false;
    }

    UE_LOG(LogTemp, Warning, TEXT("Dodge Excuted!"));
    // 문제가 없다면 회피 호출 후 true 리턴
    DodgeStart(Time);
    return true;
}

void APlayerBase::DodgeStart(float Time)
{   
    // 부모함수 호출
    Super::DodgeStart(Time);

    // 실행 시 선입력 플래그 해제
    bSaveDodge = false;

    if (!MovementComp)
    {
        return;
    }

    // 회피 중 입력 제한
    bIsMoveLockedWhileDodging = true;

    // 지면마찰력을 0으로 변경
    MovementComp->GroundFriction = 0.0f;

    // 캐릭터 전방 벡터 구하기
    FVector ForwardDir = GetActorForwardVector();   

    // 전진하는 힘과 방향 계산
    FVector DashVelocity = ForwardDir * DodgeVelocity;

    // Z값을 현재 플레이어의 Z값으로 바꾸고 적용
    DashVelocity.Z = MovementComp->Velocity.Z;
    MovementComp->Velocity = DashVelocity;
}

void APlayerBase::DodgeEnd()
{
    Super::DodgeEnd();

    if (!MovementComp)
    {
        return;
    }

    MovementComp->GroundFriction = SavedGroundFriction;
}

void APlayerBase::ResetDodgeCooldown()
{
    Super::ResetDodgeCooldown();

    if (bSaveDodge)
    {
        UE_LOG(LogTemp, Warning, TEXT("ResetDodgeCooldown: bSaveDodge"));
    }
    if (!bIsAttacking)
    {
        UE_LOG(LogTemp, Warning, TEXT("ResetDodgeCooldown: !bSIsAttacking"));
    }
    if (IsCharacterCanAction())
    {
        UE_LOG(LogTemp, Warning, TEXT("ResetDodgeCooldown: IsCharacterCanAction()"));
    }



    // 선입력된 회피 입력이 있다면 즉시 회피 시작
    if (bSaveDodge && !bIsAttacking && IsCharacterCanAction())
    {   
        UE_LOG(LogTemp, Warning, TEXT("Buffered Dodge Excuted!"));
        bSaveDodge = false;
        DodgeStart(DodgeDuration);
    }
}

void APlayerBase::UnlockMoveInputAfterDodge()
{
    bIsMoveLockedWhileDodging = false;
}

bool APlayerBase::IsCharacterCanAction()
{
    bool bIsCanAct = Super::IsCharacterCanAction();

    return bIsCanAct;
}
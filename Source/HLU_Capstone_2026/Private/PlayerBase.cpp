#include "PlayerBase.h"
#include "BlueprintGameplayTagLibrary.h"
#include "GameplayTagContainer.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PaperFlipbookComponent.h"

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

    // 캐릭터 컴포넌트에서 앉기 기능 활성화
    GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true;

    // 캐릭터가 않은 상태에서도 절벽 아래로 떨어질 수 있도록 조정(슬라이딩 시 떨어짐 허용)
    GetCharacterMovement()->bCanWalkOffLedgesWhenCrouching = true;
}

void APlayerBase::BeginPlay()
{
    Super::BeginPlay();

    // 플레이어 최대 이동속도 저장
    SavedPlayerMaxWalkSpeed = GetCharacterMovement()->MaxWalkSpeed;
}

void APlayerBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (MovementComp)
    {   
        ChangeGravity();

        // 벽타기 기능 함수
        if (bCanClimbWall)
        {
            CheckWall();
        }
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
    //UE_LOG(LogTemp, Warning, TEXT("C++ First Attack Started"));
    Attack();
}

void APlayerBase::Attack_Implementation()
{   
    // 부모 Attack 함수 미실행
    //Super::Attack_Implementation();

    // 애니메이션 재생 전 방향키로 입려된 방향으로 몸을 돌림
    UpdateFacingDirection();

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

void APlayerBase::AirDefaultAttack()
{
    //UE_LOG(LogTemp, Warning, TEXT("Air Default Attack Called"));

    // 공중 일반공격 애니메이션 재생
    PlayDefaultAirAttackAnimation();
}

void APlayerBase::AirDownwardAttack()
{   
    //UE_LOG(LogTemp, Warning, TEXT("Air Downward Attack Called"));

    // 공중 공격 시 약간의 공중 체공 기능
    /*FVector CurrentVelocity = MovementComp->Velocity;
    CurrentVelocity.Z = 0.0f;
    MovementComp->Velocity = CurrentVelocity;*/

    // 공중 하단공격 애니메이션 재생 
    PlayDownwardAirAttackAnimation();

    //UE_LOG(LogTemp, Warning, TEXT("Air Attack Executed! Suspension applied."));
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

    // 가드 선입력이 있는지 판단, 가드 선입력이 눌려질 시 선입력된 회피/공격은 취소
    if (bSaveGuard && bCanGuard)
    {   
        //UE_LOG(LogTemp, Warning, TEXT("Guard Input Buffered Excuted!"));
        bSaveGuard = false;
        bSaveAttack = false;
        bSaveDodge = false;

        // 즉시 공격상태 초기화
        EndAttackState();

        // 모든 콤보 변수 초기화
        FullResetCombo();

        // 가드 시작
        GuardStart();
        return;
    }

    // 회피 선입력이 있는지 판단, 회피 선입력이 눌려질 시 선입력된 공격은 취소
    if (bSaveDodge && bCanDodge)
    {   
        //UE_LOG(LogTemp, Warning, TEXT("Dodge Input Buffered After Attack!"));
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

    //UE_LOG(LogTemp, Warning, TEXT("Combo State End"));

    // SecondAttackWaitTime 이후에 모든 콤보 플래그 리셋 함수를 호출
    //UE_LOG(LogTemp, Warning, TEXT("C++ Player Wait Next Input, WaitTime: %.2f"), SecondAttackWaitTime);
    GetWorldTimerManager().SetTimer(ComboTimerHandle, this, &APlayerBase::FullResetCombo, SecondAttackWaitTime, false);
}

void APlayerBase::EndAttackState()
{   
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
    
    // 글로벌에 n초간 히트스탑
    ApplyHitStopGlobal(HitStopTime, HitStopDilation);

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

            //UE_LOG(LogTemp, Warning, TEXT("C++: Hit Stun & Invincible Ended"));
        }), HitInvincibleTime, false);

    return true;
}

void APlayerBase::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode)
{
    // 1. 부모 클래스의 기본 로직 실행
    Super::OnMovementModeChanged(PrevMovementMode, PreviousCustomMode);

    // 걷다가 허공으로 떨어지기 시작했을 경우 실행
    if (PrevMovementMode == MOVE_Walking && GetCharacterMovement()->MovementMode == MOVE_Falling)
    {
        if (bIsCrouched)
        {   
            // 절벽에서 떨어졌을 때 선입력 플래그 모두 초기화
            bSaveDodge = false;
            bSaveJump = false;  
             
            // 튕겨나가는 속도 0.7배로 깍기
            FVector CurrentVelocity = MovementComp->Velocity;
            CurrentVelocity.X *= GroundDodgeDecelerateCoefficient;
            MovementComp->Velocity = CurrentVelocity;
            
            // 애니메이션 강제종료(ABP가 제어하는 애니메이션으로 복귀)
            // StopAnimationOverride();
        }
    }
}

void APlayerBase::TryJump()
{
    // 1. 컴포넌트 유효성 검사
    if (!MovementComp)
    {
        return;
    }

    // 벽에서 점프가 호출되었을 경우
    if (bIsOnWall)
    {
        bIsOnWall = false;

        // 입력 잠금 활성화
        bIsWallJumpInputLocked = true;

        // 일정시간 후 입력 잠금 해제
        GetWorldTimerManager().ClearTimer(WallJumpLockoutTimerHandle);
        GetWorldTimerManager().SetTimer(WallJumpLockoutTimerHandle, this, &APlayerBase::ReleaseWallJumpLock, WallJumpLockoutDuration, false);

        // 벽 점프 힘 설정 (위로 띄우는 힘 + 벽 반대편으로 밀어내는 힘)
        float WallJumpZForce = MovementComp->JumpZVelocity;
        float WallJumpPushForce = 600.0f;   // 반대편으로 밀어내는 수평 힘

        // 점프 방향 최종 결정
        FVector JumpForce = (CurrentWallNormal * WallJumpPushForce) + FVector(0.0f, 0.0f, WallJumpZForce);

        // 캐릭터 방향도 강제로 벽 반대편으로 돌림
        SetActorRotation(CurrentWallNormal.Rotation());

        // 캐릭터 발사
        LaunchCharacter(JumpForce, true, true);

        bIsJumping = true;
        CurrentJumpCount = 1;

        //UE_LOG(LogTemp, Warning, TEXT("Wall Jump!"));
        return;
    }

    // 2. 점프가 불가능한 상황에서는 점프 불가
    if (!IsCharacterCanAction() || bIsAttacking)
    {
        // 회피 도중 호출될 시 선입력 저장 
        if (bIsDodging)
        {
            bSaveJump = true;
            //UE_LOG(LogTemp, Warning, TEXT("Jump Buffered during Dash!"));

            FTimerHandle BufferTimer;
            GetWorldTimerManager().SetTimer(BufferTimer, FTimerDelegate::CreateLambda([this]() {
                bSaveDodge = false;
                //UE_LOG(LogTemp, Warning, TEXT("Jump Buffered Cancelled"));
                }), 0.2f, false);
        }

        return;
    }

    // 3. 함수 호출 당시의 수직 제외 수평 속도만 추출 및 최대 허용 속도 지정
    FVector HorizontalVelocity = FVector(MovementComp->Velocity.X, MovementComp->Velocity.Y, 0.0f);
    float MaxAllowedJumpSpeed = SavedPlayerMaxWalkSpeed * 1.15f;    // 허용할 점프 최대 속도

    // UE_LOG(LogTemp, Warning, TEXT("Jump Tried!"));

    // 4. 플레이어 적용 속도 계산
    if (HorizontalVelocity.Size() > MaxAllowedJumpSpeed)
    {
        // 속도와 방향을 유지한 채, 크기만 최대 허용치로 줄이기
        HorizontalVelocity = HorizontalVelocity.GetSafeNormal() * MaxAllowedJumpSpeed;

        // 조절된 수평 속도에 기존 Z축 속도를 합쳐서 덮어씌우기
        MovementComp->Velocity = FVector(HorizontalVelocity.X, HorizontalVelocity.Y, MovementComp->Velocity.Z);

        //UE_LOG(LogTemp, Warning, TEXT("Dash-Jump Speed Clamped to %f!"), MaxAllowedJumpSpeed);
    }

    // 5. 이단점프 실행 로직
    if (MovementComp->IsFalling())
    {
        if (CurrentJumpCount < MaxJumpCount)
        {
            ExcuteDoubleJump();
            return;
        }
    }
    // 6. 기본 점프(지면에서의) 실행 로직
    else
    {
        bIsJumping = true;  // 점프 플래그 활성화

        // 만약 Crouched때문에 엔진 내부에서 점프가 안될 경우, Lauch로 캐릭터 강제 점프
        if (bIsCrouched)
        {
            UnCrouch();

            //UE_LOG(LogTemp, Warning, TEXT("Jump Start! (LauchCharacter)"));
            FVector JumpForce = FVector(0.f, 0.f, MovementComp->JumpZVelocity);
            LaunchCharacter(JumpForce, false, true);

            return;
        }

        //UE_LOG(LogTemp, Warning, TEXT("Jump Start!"));
        CurrentJumpCount = 1;
        Jump();
    }
}

void APlayerBase::TryStopJumping()
{   
    // 점프 중단, 추후 별도 로직 추가 가능
    StopJumping();
}

void APlayerBase::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    // 공중 공격 도중 착지하였을 경우
    if (bIsAttacking)
    {
        EndAttackState();

        FullResetCombo();

        StopAnimationOverride();
    }

    // 점프 플래그 비활성화
    bIsJumping = false;

    if (bSaveDodge)
    {
        bSaveDodge = false;

        DodgeStart(DodgeDuration);

        //UE_LOG(LogTemp, Warning, TEXT("Landing Dash Executed by Buffer!"));
    }

    // 점프 횟수 초기화
    CurrentJumpCount = 0;

    // 벽타기 플래그 초기화
    bIsOnWall = false;

    // TODO: 이펙트, 사운드, 하드랜딩 분기
}

void APlayerBase::ExcuteDoubleJump()
{
    CurrentJumpCount++;

    // 추락 관성 죽이기
    FVector CurrentVelocity = MovementComp->Velocity;
    CurrentVelocity.Z = 0.0f;
    MovementComp->Velocity = CurrentVelocity;

    // 강제로 위로 쏘아올리기
    FVector JumpForce = FVector(0.f, 0.f, MovementComp->JumpZVelocity * 1.1f);
    LaunchCharacter(JumpForce, false, true);

    bIsJumping = true;
    // StopAnimationOverride();

    UE_LOG(LogTemp, Warning, TEXT("Double Jump Executed! Current Count: %d"), CurrentJumpCount);
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
    float RunThreshold = MovementComp->MaxWalkSpeed * 0.9f;

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

    // 만약 회피가 불가능한 상황이면 회피하지 않고 false 리턴
    if (!IsCharacterCanAction())
    {   
        //UE_LOG(LogTemp, Warning, TEXT("Dodge Return"));
        return false;
    }

    // 공격 도중 선입력이 있었거나 회피 불가능한 상황에서 입력이 있었을 경우
    if (bIsAttacking || !bCanDodge || MovementComp->IsFalling())
    {   
        // 회피 선입력 트리거를 활성화
        bSaveDodge = true;
        //UE_LOG(LogTemp, Warning, TEXT("Dodge Input Buffered"));

        // 0.n초 뒤 예약 자동 해제
        FTimerHandle BufferTimer;
        GetWorldTimerManager().SetTimer(BufferTimer, FTimerDelegate::CreateLambda([this]() {
                bSaveDodge = false;
                //UE_LOG(LogTemp, Warning, TEXT("Dodge Input Buffer Canceled"));
            }), 0.25f, false);

        // 회피가 실행되지 않았으니 false 리턴
        return false;
    }

    //UE_LOG(LogTemp, Warning, TEXT("Dodge Excuted!"));
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

    // 플레이어의 입력 방향으로 즉시 전환
    UpdateFacingDirection();

    // 회피 중 입력 제한
    bIsMoveLockedWhileDodging = true;
}

void APlayerBase::DodgeEnd()
{
    Super::DodgeEnd();

    if (!MovementComp)
    {
        return;
    }

    if (bSaveJump)
    {
        bSaveJump = false;

        // 애니메이션 강제종료, 노티파이 스테이트의 UnCrouch 강제호출
        StopAnimationOverride();

        //UE_LOG(LogTemp, Warning, TEXT("Buffered Jump Executed!"));

        // 점프 실행
        TryJump();

        return;
    }

    // 캐릭터가 지면에 있다면, 대시 직후 속도를 평소보다 높게 설정
    if (!MovementComp->IsFalling())
    {   
        MovementComp->MaxWalkSpeed = DodgeVelocity * 1.1;
    }
    
    // 일정 시간 이후 속도를 줄이는 타이머
    GetWorldTimerManager().SetTimer(MomentumTimerHandle, this, &APlayerBase::DecelerateMomentum, 0.1f, true);

}

void APlayerBase::ResetDodgeCooldown()
{
    Super::ResetDodgeCooldown();

    // 선입력된 회피 입력이 있다면 즉시 회피 시작
    if (bSaveDodge && !bIsAttacking && IsCharacterCanAction())
    {   
        //UE_LOG(LogTemp, Warning, TEXT("Buffered Dodge Excuted!"));
        bSaveDodge = false;
        DodgeStart(DodgeDuration);
    }
}

void APlayerBase::AddVelocityWhileDodging()
{
    // 지면마찰력을 0으로 변경
    MovementComp->GroundFriction = 0.0f;

    // 캐릭터 전방 벡터 구하기
    FVector ForwardDir = GetActorForwardVector();

    // 전진하는 힘과 방향 계산
    FVector DashVelocity = ForwardDir * DodgeVelocity;

    // Z값을 현재 플레이어의 Z값으로 바꾸고 적용
    DashVelocity.Z = MovementComp->Velocity.Z;

    // 만약 공중에 있을 경우 강제 감속
    if (MovementComp->IsFalling())
    {
        DashVelocity.X *= GroundDodgeDecelerateCoefficient;
    }

    MovementComp->Velocity = DashVelocity;

    //UE_LOG(LogTemp, Warning, TEXT("Dash Velocity Added, X: %.2f, Y: %.2f"), DashVelocity.X, DashVelocity.Y);
}

void APlayerBase::DecelerateMomentum()
{   
    if (!MovementComp)
    {
        return;
    }

    float NormalRunSpeed = SavedPlayerMaxWalkSpeed;
    float CurrentMaxSpeed = MovementComp->MaxWalkSpeed;

    if (CurrentMaxSpeed > NormalRunSpeed)
    {
        MovementComp->MaxWalkSpeed = FMath::FInterpTo(CurrentMaxSpeed, NormalRunSpeed, 0.1f, 5.0f);
    }
    else
    {
        MovementComp->MaxWalkSpeed = NormalRunSpeed;
        GetWorldTimerManager().ClearTimer(MomentumTimerHandle);
    }
}

void APlayerBase::UnlockMoveInputAfterDodge()
{
    bIsMoveLockedWhileDodging = false;
}

void APlayerBase::UpdateFacingDirection()
{   
    // 입력 데드존 판단, 근데 키보드는 딱히 의미 없음
    if (FMath::Abs(CurrentRawInputX) > 0.1f)
    {   
        // 입력이 양수면 0도, 음수면 180도 회전
        FRotator TargetRot = (CurrentRawInputX > 0.f) ? FRotator(0.f, 0.f, 0.f) : FRotator(0.f, 180.f, 0.f);

        SetActorRotation(TargetRot);
    }
}

void APlayerBase::StopMoveInstantly()
{   
    // 이동 즉시 중단
    if (MovementComp)
    {
        MovementComp->Velocity = FVector::ZeroVector;
    }
}

void APlayerBase::StartSlideCapsule()
{
    Crouch();
    //UE_LOG(LogTemp, Warning, TEXT("Slide Capsule Reduced!"));
}

void APlayerBase::EndSlideCapsule()
{   
    // 놀랍게도 천장에 막혀있으면 엔진이 기다렸다 알아서 일어나게 해줌
    UnCrouch();
    //UE_LOG(LogTemp, Warning, TEXT("Slide Capsule Restored!"));
}

void APlayerBase::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    // 부모 클래스의 기본 로직 실행
    Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    // 카메라 및 스프라이트 위치보정값
    float FixHeightAdjust = HalfHeightAdjust * 0.95;

    // 캐릭터 스프라이트 위치 보정
    if (UPaperFlipbookComponent* VisualComp = GetSprite())
    {
        VisualComp->AddLocalOffset(FVector(0.0f, 0.0f, FixHeightAdjust));
    }

    // 스프링 암 보정
    if (USpringArmComponent* SpringArm = FindComponentByClass<USpringArmComponent>())
    {
        SpringArm->AddLocalOffset(FVector(0.0f, 0.0f, FixHeightAdjust));
    }

    //UE_LOG(LogTemp, Warning, TEXT("Crouch Started: Mesh moved UP by %f"), FixHeightAdjust);
}

void APlayerBase::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    // 부모 클래스 로직 실행
    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    // 카메라 및 스프라이트 위치보정값
    float FixHeightAdjust = HalfHeightAdjust * 0.95 * -1;

    if (UPaperFlipbookComponent* VisualComp = GetSprite())
    {
        VisualComp->AddLocalOffset(FVector(0.0f, 0.0f, FixHeightAdjust));
    }

    if (USpringArmComponent* SpringArm = FindComponentByClass<USpringArmComponent>())
    {
        SpringArm->AddLocalOffset(FVector(0.0f, 0.0f, FixHeightAdjust));
    }

    //UE_LOG(LogTemp, Warning, TEXT("Crouch Ended: Mesh moved DOWN by %f"), FixHeightAdjust);
}

void APlayerBase::ChangeGravity()
{
    // 공중에 떠있는 경우에만 해당 함수 로직 실행
    if (MovementComp->IsFalling())
    {   
        // z속도가 0보다 작다 = 점프에서 최대높이에 도달하고 낙하 시작
        if (MovementComp->Velocity.Z < 0.0f)
        {   
            // 낙하중일때 2배 강하게 끌어당김
            MovementComp->GravityScale = 2.0f;
        }
        else
        {   
            // 상승중일땐 기본 1 유지
            MovementComp->GravityScale = 1.0f;
        }
    }
    else
    {
        MovementComp->GravityScale = 1.0f;
    }
}

bool APlayerBase::TryGuard()
{   
    // 부모 함수 미호출
    //bool bCanGuard = Super::TryGuard();

    // 1. 행동 불가능한 경우 즉시 리턴
    if (!IsCharacterCanAction())
    {
        return false;
    }

    // 2. 가드 선입력 판정
    if (bIsAttacking || !bCanGuard)
    {   
        bSaveGuard = true;

        FTimerHandle BufferTimer;
        GetWorldTimerManager().SetTimer(BufferTimer, FTimerDelegate::CreateLambda([this]() {
            bSaveGuard = false;
        }), GuardBufferTime, false);
        
        //UE_LOG(LogTemp, Warning, TEXT("Guard Input Buffered!"));

        // 가드 안됬으니 false 리턴
        return false;
    }

    //UE_LOG(LogTemp, Warning, TEXT("Player Guard Excuted!"));
    // 문제 없으면 가드 실행 및 true 리턴 
    GuardStart();
    return true;
}

void APlayerBase::GuardStart()
{   
    // 부모 로직 먼저 실행
    Super::GuardStart();

    // 실행시 선입력 예약 해제
    bSaveGuard = false;

    // 플레이어의 입력 방향으로 즉시 전환
    UpdateFacingDirection();

    // 가드 도중 마찰력 크게 증가
    if (MovementComp)
    {
        MovementComp->GroundFriction = 10.0f;
    }
}

void APlayerBase::EndGuard()
{
    Super::EndGuard();

    //UE_LOG(LogTemp, Warning, TEXT("C++ PlayerBase: EndGuardCalled!"));
}

void APlayerBase::ResetGuardCooldown()
{
    Super::ResetGuardCooldown();

    if (bSaveGuard && !bIsAttacking && IsCharacterCanAction())
    {
        //UE_LOG(LogTemp, Warning, TEXT("C++ PlayerBase: Excute Buffered Guard After Guard"));
        GuardStart();
    }
}

void APlayerBase::CheckWall()
{
    if (!MovementComp->IsFalling() || !IsCharacterCanAction())
    {
        if (bIsOnWall)
        {
            bIsOnWall = false;
        }
        return;
    }

    FHitResult HitResult;
    // 캐릭터 위치
    FVector StartLocation = GetActorLocation();
    // 캐릭터 전방 50cm 거리
    FVector EndLocation = StartLocation + (GetActorForwardVector() * 50.0f);

    // 라인 트레이스 발사 (Visibility 채널로 검사 )
    bool bHit = GetWorld()->LineTraceSingleByChannel(
        HitResult, StartLocation, EndLocation, ECC_Visibility
    );

    // 레이저에 뭐가 맞으면
    if (bHit && HitResult.GetActor()->ActorHasTag(FName("Climbable")))
    {
        // UE_LOG(LogTemp, Warning, TEXT("Climbable Detected!"));
        // 벽의 수직방향과 플레이어 전방 방향의 내적
        float DotProduct = FVector::DotProduct(GetActorForwardVector(), HitResult.Normal);

        if (DotProduct < -0.5f && MovementComp->Velocity.Z <= 400.0f) // 떨어지는 중일때 매달림
        {
            if (!bIsOnWall)
            {   
                bIsOnWall = true;
                CurrentWallNormal = HitResult.Normal;   // 벽에서의 반발 방향 저장
                CurrentJumpCount = 1;   // 점프 제한

                //UE_LOG(LogTemp, Warning, TEXT("Grabbed the Wall!"));
            }

            // 벽에서 미끌어져 내려오게 만들기
            FVector CurrentVelocity = MovementComp->Velocity;   // 현재 이동속도 저장
            CurrentVelocity.Z = -WallSlideSpeed;
            CurrentVelocity.X = 0.0f;
            MovementComp->Velocity = CurrentVelocity;

            return;
        }
    }

    if (bIsOnWall)
    {
        bIsOnWall = false;
        //UE_LOG(LogTemp, Warning, TEXT("Detached from Wall"));
    }
}

float APlayerBase::FilterInputWhileOnWall(float MovementVectorX)
{   
    FVector ReturnValue = FVector::Zero();

    // 벽을 타고 있을때만 동작
    if (bIsOnWall)
    {   
        // 두 값의 부호가 다름 = 벽 방향으로 방향키 입력중
        if (MovementVectorX * CurrentWallNormal.X < 0.0f)
        {
            return 0;
        }
        // 두 값의 부호가 같음 = 벽 반대 방향으로 방향키 입력중
        else if(MovementVectorX * CurrentWallNormal.X > 0.0f)
        {
            bIsOnWall = false;
            return MovementVectorX;
        }
    }

    return MovementVectorX;
}

bool APlayerBase::IsCharacterCanAction()
{
    bool bIsCanAct = Super::IsCharacterCanAction();

    return bIsCanAct;
}
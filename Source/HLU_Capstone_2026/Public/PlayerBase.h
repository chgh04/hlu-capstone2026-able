#pragma once

#include "CoreMinimal.h"
#include "DefaultCharBase.h"
#include "CheckpointInteractable.h"
#include "InventoryComponent.h"
#include "PlayerInteractComponent.h"
#include "InteractReceiver.h"
#include "PilgrimSaveGame.h"
#include "SaveLoadComponent.h"
#include "PlayerBase.generated.h"

/**
 플레이어 캐릭터의 베이스 클래스입니다. 

 ADefaultCharBase 클래스를 상속받으며,
 HealthComponent와의 통신을 위해 IDamageable 인터페이스를 구현합니다. 
 */

// 전방 선언
class AInteractableBase;

UCLASS()
class HLU_CAPSTONE_2026_API APlayerBase : public ADefaultCharBase, public ICheckpointInteractable, public IInteractReceiver
{
	GENERATED_BODY()

public:
    APlayerBase();

protected:
    virtual void BeginPlay() override;

    virtual void Tick(float DeltaTime) override;

// 인터페이스 구현 --------------------------------------
public:
    // 체크포인트 휴식 시 호출될 함수
    virtual void RestAtCheckpoint_Implementation(float HealPercentage, AActor* CheckpointRef) override;
    virtual void SaveAtCheckpoint_Implementation(FVector CheckpointLocation) override;

    // IInteractReceiver - 아이템 등록/해제
    virtual void RegisterNearbyItem_Implementation(AActor* Item) override;
    virtual void UnregisterNearbyItem_Implementation(AActor* Item) override;

    // IInteractReceiver - 인터랙터블 등록/해제
    virtual void RegisterNearbyInteractable_Implementation(AActor* Interactable) override;
    virtual void UnregisterNearbyInteractable_Implementation(AActor* Interactable) override;

    // BP_Player의 IA_Interact(F키) Started에서 호출
    // 기존 Get All Actors Of Class 두 개를 이 함수 하나로 대체
    UFUNCTION(BlueprintCallable, Category = "Interaction")
    void HandleInteractInput();

// 컴포넌트 생성 --------------------------------------
protected:
    // 카메라 암
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class USpringArmComponent> CameraString;

    // 실제 카메라
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class UCameraComponent> MainCamera;

    // 인벤토리 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UInventoryComponent* InventoryComponent;

    // 세이브/로드 및 체크포인트 활성화 목록 관리 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USaveLoadComponent* SaveLoadComponent;

    // 플레이어 상호작용 관리 컴포넌트 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPlayerInteractComponent* InteractComponent;

    // 지속형 나이아가라 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player_VFX")
    class UNiagaraComponent* PlayerTrackingNiagaraVFX;

    // 플레이어를 은은하게 비추는 포인트 라이트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player_VFX")
    class UPointLightComponent* PlayerLight;

// 공격 기능 함수/변수 --------------------------------------
protected:
    // 상속받은 TryAttack 오버라이드
    virtual void TryAttack() override;
    
    // 상속받은 Attack_Implemetation 함수 구현
    virtual void Attack_Implementation() override;

    // 공격 대상을 구분하는 함수(적) 구체화
    virtual bool CanAttackTarget(AActor* Target) const;

    // 공격 적중 시 플레이어 영향 
    virtual void ExecuteAttackHit(AActor* TargetActor, TSubclassOf<class UCustomDamageType> DamageType, float DamageMultiplier = 1.0f) override;

    // HitStop(역경직) 적용 시간
    UPROPERTY(EditDefaultsOnly, Category = "Player_Combat_Attack")
    float HitStopTime = 0.05f;

    // HitStop(역경직) 감속 계수
    UPROPERTY(EditDefaultsOnly, Category = "Player_Combat_Attack")
    float HitStopDilation = 0.05f;

    // 공격 시 전진 거리
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player_Combat_Attack")
    float AttackStepForce = 500.f;

    // AttackStepForce 반환 함수
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player_Combat_Attack")
    float GetAttackStepForce() { return AttackStepForce; }

    // 달리는 도중 공격 시 전진거리 배율
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player_Combat_Attack")
    float AttackStepForceMultiplierWhileRun = 2.0f;

    // 공격 시작 전 당시의 속도 저장 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player_Combat_Attack")
    float SavedAttackSpeed;

    UFUNCTION(BlueprintCallable, Category = "Player_Combat_Attack")
    // 공중일반공격 수행 함수
    void AirDefaultAttack();

    UFUNCTION(BlueprintCallable, Category = "Player_Combat_Attack")
    // 공중아래공격 수행 함수
    void AirDownwardAttack();

    // 플레이어 공격 범위 기본값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player_Combat_Attack")
    FVector DefaultAttackBoxSize = FVector(72.f, 48.f, 72.f);

    // 플레이어 공격 박스 상대위치 기본값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player_Combat_Attack")
    FVector DefaultAttackBoxLoc = FVector(50.f, 0.f, 0.f);

    // 플레이어 체공 하단 공격 범위
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player_Combat_Attack")
    FVector AirDownwardAttackBoxSize = FVector(96.f, 48.f, 72.f);

    // 플레이어 체공 하단 공격 공격 박스 상대위치
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player_Combat_Attack")
    FVector AirDownwardAttackBoxLoc = FVector(0.f, 0.f, -20.f);

    // 플레이어 공중 공격시 공격 박스 조절 함수
    UFUNCTION(BlueprintCallable, Category = "Player_Combat_Attack")
    void AttackBoxExtendStart();

    // 플레이어 공중 공격 종료시 박스 사이즈 초기화 함수
    UFUNCTION(BlueprintCallable, Category = "Player_Combat_Attack")
    void AttackBoxExtendEnd();

    // 플레이어의 공중일반공격 애니메이션 재생 함수
    UFUNCTION(BlueprintImplementableEvent, Category = "Player_Combat_Attack")
    void PlayDefaultAirAttackAnimation();

    // 플레이어의 공중하단공격 애니메이션 재생 함수
    UFUNCTION(BlueprintImplementableEvent, Category = "Player_Combat_Attack")
    void PlayDownwardAirAttackAnimation();

    // 플레이어의 모든 공중 공격 플래그
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player_Combat_Attack")
    bool bIsAirAttacking = false;

    // 플레이어가 공중 하단공격을 실행중인지 표시
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player_Combat_Attack")
    bool bIsAirDownwardAttacking = false;

public:
    // 플레이어의 애니메이션 강제종료 함수
    UFUNCTION(BlueprintImplementableEvent, BlueprintCallable, Category = "Player_Animation")
    void StopAnimationOverride();

// 플레이어 콤보 공격 관련 --------------------------------------
protected:
    // 플레이어 콤보 연계가 가능하도록 전환 (ABP의 노티파이에서 호출함)
    UFUNCTION(BlueprintCallable, Category = "Player_Combat_Attack")
    void CheckCombo();

    // 플레이어의 콤보공격이 연계/취소되었을때 및 공격 도중의 선입력 관리
    UFUNCTION(BlueprintCallable, Category = "Player_Combat_Attack")
    void ResetCombo();

    // 현재 플레이어의 공격 타수
    UPROPERTY(BlueprintReadOnly, Category = "Player_Combat_Attack")
    int32 ComboCount = 0;

    // 공격 연계 타이밍 이전의 연계 입력은 무시하기 위한 트리거
    UPROPERTY(BlueprintReadOnly, Category = "Player_Combat_Attack")
    bool bIgnoreSaveAttack = true;

    // 공격 도중 버튼을 또 눌렀는지 판단, 애니메이션 노티파이와 함께 사용(AS_Player), 애니메이션 도중의 입력 판단에 사용됩니다. 
    UPROPERTY(BlueprintReadOnly, Category = "Player_Combat_Attack")
    bool bSaveAttack = false; 

    // 첫 번째 공격 이후 몇초간 콤보를 이어갈 입력을 받는지 정의
    UPROPERTY(EditDefaultsOnly, Category = "Player_Combat_Attack")
    float SecondAttackWaitTime = 0.2f;

    // 첫 번째 공격 이후 공격 대기상태 판단, 애니메이션 종료 후 입력 판단에 사용됩니다. 
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player_Combat_Attack")
    bool bIsWaitNextAttackInput = false;

    // 일반공격/피격 이후 상태 복구 함수 (부모 클래스 함수 재사용), 각 공격에 대한 종료를 정의
    virtual void EndAttackState() override;

    // Idle<->AttackWait 상태전환 전달용 함수
public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player_Combat_Attack")
    bool IdleAttackWaitTrasitionFlag();

private:
    // 콤보 초기화 변수가 포함된 함수
    void FullResetCombo();

// 피격 관련 함수/변수 --------------------------------------
protected:
    // 부모클래스에서 상속받아 사용
    virtual bool GetHit(const FDamageData& DamageData) override;

    // 플레이어 피격무적 시간 변수
    UPROPERTY(EditDefaultsOnly, Category = "Player_Combat_Hit")
    float HitInvincibleTime = 1.0f;

// 플레이어 이동/회피 관련 함수/변수 --------------------------------------
private:
    float SavedPlayerMaxWalkSpeed;

protected:
    // 캐릭터의 이동모드(걷기, 낙하등)가 변경될 때 호출되는 함수 
    virtual void OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PreviousCustomMode) override;

    // 플레이어의 최대 점프 가능 횟수
    UPROPERTY(EditDefaultsOnly, Category = "Player_Movement")
    bool bCanDoubleJump = false;

    // 현재 플레이어의 점프 횟수 카운트
    int32 CurrentJumpCount = 0; 

    // 점프 시도 함수, 지면에서의 점프인지 공중에서의 점프인지 판단
    UFUNCTION(BlueprintCallable, Category = "Player_Movement")
    virtual void TryJump();

    // 점프 중단(점프 버튼을 손에서 뗏을 때)
    UFUNCTION(BlueprintCallable, Category = "Player_Movement")
    virtual void TryStopJumping();

    // 캐릭터가 땅에 닿는 순간 엔진이 자동 호출
    virtual void Landed(const FHitResult& Hit) override;
    
    // 이단점프 실행 함수
    void ExcuteDoubleJump();

    // 애니메이션 노티파이에서 호출할 전진 스텝 함수 
    UFUNCTION(BlueprintCallable, Category = "Player_Combat_Attack")
    void StepForward(float StepForce = 200.0f);

    // 플레이어가 점프를 눌렀는지에 대한 플래그
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player_Movement")
    bool bIsJumping = false;

    // 플레이어의 점프 선입력 플래그
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player_Movement")
    bool bSaveJump = false;

    // 플레이어 회피 시 전진성
    UPROPERTY(EditDefaultsOnly, Category = "Player_Combat_Dodge")
    float DodgeVelocity = 500.0f;
    
    // 플레이어 공중대시 플래그 
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player_Combat_Dodge")
    bool bCanAirDash = false;

    // 플레이어의 기본 중력 적용값
    float PlayerGravity = 1.0f;

    // 플레이어 공중대시 최대 사용 횟수
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player_Combat_Dodge")
    int32 MaxAirDashCount = 1;

    // 플레이어의 현재 공중대시 사용 횟수
    int32 CurrentAirDashCount = 0;

    // 플레이어 회피시도함수 재정의
    virtual bool TryDodge(float Time) override;

    // 지상 대시 시도 함수
    bool TryGroundDodge(float Time);

    // 플레이어 지상회피함수 재정의
    virtual void DodgeStart(float Time) override;

    // 플레이어 지상회피종료함수 재저의
    virtual void DodgeEnd() override;

    // 공중 대시 시도 함수
    bool TryAirDash();

    // 공중 대시 시작 함수
    void AirDashStart();

    // 공주 대시 종료 함수
    void AirDashEnd();

    // 회피 쿨타임 초기화 함수 재정의
    virtual void ResetDodgeCooldown() override;

    // 플레이어 회피 추진력 전달 함수 
    UFUNCTION(BlueprintCallable, Category = "Player_Combat_Dodge")
    void AddVelocityWhileDodging();

    // 회피 애니메이션 중 입력 제한 플래그
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player_Combat_Dodge")
    bool bIsMoveLockedWhileDodging = false;

    // 회피 선입력 예약 플래그
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player_Combat_Dodge")
    bool bSaveDodge = false;

    // 플레이어 회피 이후 속력 유지 및 감속 타이머, 대시 이후 플레이어의 속도를 잠시 증가시키고 천천히 감속시키는 타이머에 사용됨
    void DecelerateMomentum();

    // 회피 이후 재입력 허용 플래그 전환 함수
    UFUNCTION(BlueprintCallable, Category = "Player_Combat_Dodge")
    void UnlockMoveInputAfterDodge();

    // 플레이어 입력값 저장 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player_Movement")
    float CurrentRawInputX = 0.0f;

    // 플레이어 회피시 공중에 떳을때의 감속계수
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Player_Movement")
    float GroundDodgeDecelerateCoefficient = 0.7f;

    // 플레이어의 방향을 즉시 전환하는 전용 함수
    void UpdateFacingDirection();

    // 플레이어 이동 즉시 중단
    UFUNCTION(BlueprintCallable, Category = "Player_Movement")
    void StopMoveInstantly();

    // 언리얼의 앉기 기능 실행 
    UFUNCTION(BlueprintCallable, Category = "Player_Movement")
    void StartSlideCapsule();

    // 앉기에서 다시 선 상태로 복구
    UFUNCTION(BlueprintCallable, Category = "Player_Movement")
    void EndSlideCapsule();

    // 앉기(슬라이딩) 시작 시 호출되는 엔진 내장 함수
    virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

    // 앉기(슬라이딩) 종료 시 호출되는 엔진 내장 함수
    virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

    // Crouch 후 크기 감소/복구 변수
    float FixHeightAdjust = 0.f;

    // 도약 및 낙하시 중력 계수 동적 변경 함수
    void ChangeGravity();

public:
    // bIsJump값 리턴
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player_Movement")
    bool GetIsJumping() { return bIsJumping; }

    // CurrentRawInputX 값 리턴
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player_Movement")
    float GetCurrentRawInputX() { return CurrentRawInputX; }

// 플레이어 가드 관련 함수/변수 --------------------------------------
protected:
    // 부모 TryGuard 재정의
    virtual bool TryGuard() override;

    // 부모 GuardStart 재정의
    virtual void GuardStart() override;

    // 부모 EndGuard 재정의
    virtual void EndGuard() override;

    // 가드 선입력 유효 시간 (회피 0.2s보다 짧은 0.1s 추천)
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player_Comat_Guard")
    float GuardBufferTime = 0.1f;

    // 가드 선입력 플래그
    bool bSaveGuard = false;

    // 부모 ResetGuardCooldown 재정의
    virtual void ResetGuardCooldown() override;

    // 플레이어의 가드 시 움직임 봉인 플래그
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player_Comat_Guard")
    bool bIsMoveLockedWhileGuarding = false;

// 플레이어 벽타기 관련 함수/변수 --------------------------------------
protected:
    // 플레이어가 벽타기가 가능한 상태인지에 대한 플래그
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player_Movement")
    bool bCanClimbWall = false;

    // 벽타기 상태 플래그
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player_Movement")
    bool bIsOnWall = false;

    // 벽에서 미끄러지는 속도
    UPROPERTY(EditDefaultsOnly, Category = "Player_Movement")
    float WallSlideSpeed = 150.0f;

    // 우리가 매달려 있는 벽의 표면 방향 (벽 점프할 때 튕겨 나갈 방향 계산용)
    FVector CurrentWallNormal;

    // 매 프레임 벽을 체크하는 함수
    void CheckWall();

    // 벽 점프 이후 입력 잠금 상태 
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player_Movement")
    bool bIsWallJumpInputLocked = false;

    UPROPERTY(EditDefaultsOnly, Category = "Player_Movement")
    float WallJumpLockoutDuration = 0.15f;

    // 입력 잠금 해제 함수
    void ReleaseWallJumpLock() { bIsWallJumpInputLocked = false; }

    // 벽을 타고있을 때 입력값 필터링 함수
    UFUNCTION(BlueprintCallable, Category = "Player_Movement")
    float FilterInputWhileOnWall(float MovementVectorX);

public:
    // 벽타기 상태 플래그 반환
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player_Movement")
    bool GetIsOnWall() { return bIsOnWall; }

// 플레이어 회복 및 부활 관련 함수/변수 --------------------------------------
protected:
    // 회복약 최대 사용 횟수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player_Potion")
    int32 MaxPotionCount = 3;

    // 현재 회복약 횟수
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player_Potion")
    int32 CurrentPotionCount;

    // 회복약 사용 시 회복량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player_Potion")
    float PotionHealAmount = 2.0f;

    // 회복약 사용 함수 (내부 키 입력 바인딩용)
    UFUNCTION(BlueprintCallable, Category = "Player_Potion")
    void UsePotion();

    // 플레이어 부활 위치
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player_Interaction")
    FVector CurrentRespawnLocation = FVector::Zero();

public:
    // 회복약 횟수 초기화 (외부 체크포인트 액터에서 접근하여 호출해야 하므로 public)
    UFUNCTION(BlueprintCallable, Category = "Player_Potion")
    void RefillPotion();

// 세이브/로드 관련 함수/변수 --------------------------------------
public:

    // 현재 포션 횟수 반환 - SaveLoadComponent에서 세이브 시 사용
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player_Save")
    int32 GetCurrentPotionCount() const { return CurrentPotionCount; }

    // 포션 횟수 설정 - SaveLoadComponent에서 로드 시 사용
    UFUNCTION(BlueprintCallable, Category = "Player_Save")
    void SetCurrentPotionCount(int32 Count) { CurrentPotionCount = Count; }

    // 리스폰 위치 설정 - SaveLoadComponent에서 로드 시 사용
    UFUNCTION(BlueprintCallable, Category = "Player_Save")
    void SetCurrentRespawnLocation(FVector Location) { CurrentRespawnLocation = Location; }

    // 게임 저장 - 체크포인트에서 호출
    UFUNCTION(BlueprintCallable, Category = "Player_Save")
    void SaveGame(FVector CheckpointLocation);

    // 게임 로드 - BeginPlay에서 세이브 파일 있으면 자동 호출
    UFUNCTION(BlueprintCallable, Category = "Player_Save")
    void LoadGame();

// 플레이어 카메라 및 상호작용 관련 함수/변수 --------------------------------------
protected:
    // 기본 카메라 스프링 암 길이 저장 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player_Interaction")
    float OriginArmLength;

    // 기본 카메라 스프링 오프셋 저장 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player_Interaction")
    FVector OriginSocketOffset;

    // 상호작용 대상의 포인터를 받아 카메라 연출 시작
    UFUNCTION(BlueprintImplementableEvent, Category = "Player_Camera")
    void PlayInteractCameraZoomIn(FVector MidpointOffset, AActor* InteractTarget);

    // 상호작용이 끝나면 원래 카메라 상태로 되돌림
    UFUNCTION(BlueprintImplementableEvent, Category = "Player_Camera")
    void PlayInteractCameraZoomOut();

    // 카메라 이동 목표가 생겼다면, 해당 목표에 대한 카메라암 길이를 저장하는 변수 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player_Camera")
    float TargetArmLength;

    // 카메라 이동 목표가 생겼다면, 해당 목표를 저장하는 변수
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player_Camera")
    FVector TargetSocketOffset;

    // 카메라 목표에 대한 이동 속도 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player_Camera")
    float CameraTransitionSpeed = 1.0f;

    // 카메라 위치 변경 지역에 들어왔을 때 호출할 함수 
    UFUNCTION(BlueprintCallable, Category = "Player_Camera")
    void SetCameraOverride(float NewArmLength, FVector NewSocketOffset);

    // 카메라 위치 변경 지역에서 나갈 때 카메라 원상복구 함수
    UFUNCTION(BlueprintCallable, Category = "Player_Camera")
    void ResetCameraOverride();

    // 매 틱마다 호출할 플레이어 카메라 보간 함수
    void UpdateCameraSettingOverride(float DeltaTime);

    // 상호작용 강제 중단(Escape)
    UFUNCTION(BlueprintCallable, Category = "Player_Interaction")
    void CancelInteraction();

// 기타 추가 기능 --------------------------------------
protected:
    // 플레이어가 이동/공격을 제한하는 이상상태에 있는지 판단하는 함수
    // 행동 가능하면 true, 행동이 불가능하면 false를 반환
    virtual bool IsCharacterCanAction() override;

    // 모든 가드/회피 플래그 초기화 함수, 공격/콤보 초기화는 FullResetCombo 및 EndAttackState에서 수행, 모든 플래그가 미리 호출되긴 하지만, 보험용으로 존재합니다. 
    virtual void ResetCombatStates() override;
    
    // 플레이어가 움직일(Walk) 수 있는 상태인지에 대한 플래그
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player_Status")
    bool IsPlayerCanMove();

    // 플레이어 스프라이트 정렬 알고리즘
    virtual void ApplySpriteSortAmount() override;

    // 플레이어 콤보 관련 타이머 관리자
    FTimerHandle ComboTimerHandle;

    // 플레이어 피격 관련 타이머 관리자
    FTimerHandle HitInvincibleTimerHandle;

    // 플레이어 회피 이후 관성 감속 타이머 관리자
    FTimerHandle MomentumTimerHandle;

    // 플레이어 벽 점프 이후 입력 잠금 타이머 관리자
    FTimerHandle WallJumpLockoutTimerHandle;

// 델리게이트 연결 --------------------------------------
private:
    // PlayerInteractComponent 카메라 연출 시작 델리게이트 연결
    UFUNCTION()
    void OnInteractStartedHandle(FVector MidpointOffset, AActor* TargetActor);

    // PlayerInteractComponent 카메라 연출 종료 델리게이트 연결
    UFUNCTION()
    void OnInteractEndedHandle();

// VFX 및 오디오
protected:
    // 회피 잔상 스폰 함수
    void SpawnGhostTrail();

    // 나이아가라 컴포넌트에서 이펙트 실행시키는 함수
    UFUNCTION(BlueprintCallable, Category = "Player_VFX")
    void PlayNiagaraCompEffect(UNiagaraSystem* NewEffect);

    // 잔상 액터
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player_VFX")
    TSubclassOf<class AGhostActor> GhostActorClass;

    // 잔상 스폰 간격
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player_VFX")
    float GhostSpawnDistnaceThreshold = 100.0f;

    // 마지막으로 잔상을 소환한 장소
    FVector LastGhostSpawnLocation = FVector::Zero();

    // 휴식시의 나이아가라 시스템 
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player_VFX")
    class UNiagaraSystem* RestCheckpointEffect = nullptr;
};

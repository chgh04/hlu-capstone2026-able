#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "Damageable.h"
#include "GameplayTagAssetInterface.h"
#include "GameplayTagContainer.h"
#include "DefaultCharBase.generated.h"

class UHealthComponent;
class UBoxComponent;

/**
 * 
 */
UCLASS()
class HLU_CAPSTONE_2026_API ADefaultCharBase : public APaperZDCharacter, public IDamageable, public IGameplayTagAssetInterface
{
	GENERATED_BODY()
	
public:
	ADefaultCharBase();

protected:
	virtual void BeginPlay() override;

// 인터페이스 구현 및 상속 -------------------
public:
	 // IDamageable 인터페이스 구현
	virtual void ReceiveDamage_Implementation(const float DamageAmount) override;

	// IDamageable 인터페이스 구현
	virtual void OnDeath_Implementation() override;

	// 외부에서 ApplyDamage 호출 시 이 함수가 자동으로 실행됨. 
	// !중요! 이 함수는 캐릭터에게 직접적인 영향을 주지 않고, 연결된 컴포넌트에 인터페이스를 통해 데이터를 전달하는 역할만 수행합니다. 
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// 태그가 저장되는 변수 **** 디테일창에서 태그 바꾸면 됨!!! ****
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player/Enemy_Combat")
	FGameplayTagContainer CharacterTags;

	// 태그 (적, 플레이어) 구분을 위한 인터페이스 함수 오버라이드 
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

// 컴포넌트 관련 -------------------
protected:
	// 체력 관리 컴포넌트 - VisibleAnywhere로 디테일 패널에서 확인 가능
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UHealthComponent* HealthComponent;

	// 공격 판정 범위 - 박스 형태의 트리거
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* AttackBox;

	// 캐릭터 MovementComponent
	class UCharacterMovementComponent* MovementComp = nullptr;

	// 캐릭터 페이퍼 플릭북 컴포넌트
	class UPaperFlipbookComponent* FlipbookComp = nullptr;

	// 캐릭터의 스프라이트 상대 위치 저장 변수(정렬을 위해 사용)
	FVector CurrentRelativeLoc;

// 공격 기능 함수/변수 -------------------
protected:
	// 공격이 시작될 때, 공격이 가능한지 확인하는 함수, 호출될 때 공격이 가능하면 공격, 아닐경우 공격하지 않습니다. 자식 클래스에서 재정의 가능
	UFUNCTION(BlueprintCallable, Category = "Player/Enemy_Combat")
	virtual void TryAttack();

	// 공격 실행 시 호출되는 이벤트
	UFUNCTION(BlueprintNativeEvent, Category = "Player/Enemy_Combat")
	void Attack();

	// 한 번 공격에 맞은 액터 목록 (중복방지)
	UPROPERTY()
	TArray<AActor*> THitActors;

	// BoxOverlap 물리효과를 QueryOnly로 전환, 일반적으로 캐릭터 ABP의 노티파이에서 함수를 호출함
	UFUNCTION(BlueprintCallable, Category = "Player/Enemy_Combat")
	void StartAttackCollision();

	// BoxOverlap 물리효과를 NoCollision으로 전환, 일반덕으로 캐릭터 ABP의 노티파이에서 함수를 호출함
	UFUNCTION(BlueprintCallable, Category = "Player/Enemy_Combat")
	void EndAttackCollision();

	// 공격 상태 종료
	UFUNCTION(BlueprintCallable, Category = "Player/Enemy_Combat")
	virtual void EndAttackState();

	// 공격 대상을 구분하는 함수(적/플레이어), OnAttackBoxOverlap에서 호출됩니다. 
	virtual bool CanAttackTarget(AActor* Target) const;

	// 공격 박스 Overlap 이벤트, 공격한 적에 대한 충돌 판정 및 필터링 수행, 일반적으로 기본 공격에 해당되는 공격에만 사용
	UFUNCTION()
	void OnAttackBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// 공격한 적에 대한 연산 수행, 피해 액터, 공격 타입, 공격 배수를 지정합니다. 기본공격 외 다양한 공격 패턴에서 호출하여 사용 가능
	UFUNCTION(BlueprintCallable, Category = "Player/Enemy_Combat")
	virtual void ExecuteAttackHit(AActor* TargetActor, TSubclassOf<class UCustomDamageType> DamageType, float DamageMultiplier = 1.0f);
	
	// 공격력 리턴
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player/Enemy_Combat")
	float GetDefaultDamage() const { return DefaultDamage; }

	// 공격력 재설정 함수
	UFUNCTION(BlueprintCallable, Category = "Player/Enemy_Combat")
	void SetDefaultDamage(float Amount);

	// 캐릭터 기본 공격력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player/Enemy_Combat")
	float DefaultDamage = 1;

	// 캐릭터가 공격 도중인지 판단
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player/Enemy_Combat")
	bool bIsAttacking = false;

	// HitStop(역경직) 및 피격 시 월드 타이머 짧게 중단(게임 전체 적용)
	UFUNCTION(BlueprintCallable, Category = "Player/Enemy_Combat")
	void ApplyHitStopGlobal(float Duration, float Dilation);

	// HitStop(역경직) 및 피격 시 액터 타이머 짧게 중단(해당 액터에게만 적용)
	UFUNCTION(BlueprintCallable, Category = "Player/Enemy_Combat")
	void ApplyHitStopCustom(float Duration, float Dilation);

	// 에디터 디테일 패널에서 선택할 수 있는 데미지 타입 변수, 기본적으로 적용되는 공격 판정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player/Enemy_Combat")
	TSubclassOf<class UCustomDamageType> CurrentAttackDamageType;

// 피해 관련 함수/변수 -------------------
protected:
	// 피격받을때(TakeDamage) 호출, 모든 피해 관련 판정을 수행, 피격이 유효했다면 true, 아니라면 false 반환
	UFUNCTION()
	virtual bool GetHit(const FDamageData& DamageData);

	// 피격시 경직되는 시간, 적은 경직 플레이어는 무적
	float StunDuration = 0.5f;

	// 피격 애니메이션 강제호출 함수(공격 함수를 덮기 위함, 넉백 면역 시 미호출) !!반드시 부모 BP가 아닌 가장 자식인 BP에서 구현해야 합니다!!
	UFUNCTION(BlueprintImplementableEvent, Category = "Player/Enemy_Animation")
	void PlayHitAnimation();

	// 넉백 적용 함수
	UFUNCTION()
	void PlayKnockBack(const FDamageData& DamageData);

	// 캐릭터가 넉백중인지 아닌지 아닌지 판단
	UPROPERTY(VisibleAnywhere, Category = "Player/Enemy_Combat")
	bool bIsKnockBack = false;

	// 캐릭터가 넉백에 면역인지 아닌지 판단
	UPROPERTY(EditAnywhere, Category = "Player/Enemy_Combat")
	bool bIsKnockBackImmune = false;

	// 캐릭터의 넉백 강도
	UPROPERTY(EditAnywhere, Category = "Player/Enemy_Combat")
	float KnockbackStrength = 500.f;

	// 캐릭터가 넉백 시 z축 넉백 강도
	UPROPERTY(EditAnywhere, Category = "Player/Enemy_Combat")
	float KnockbackZStregth = 200.f;

	// 캐릭터 사망상태 플래그
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player/Enemy_Status")
	bool bIsDead = false;

	// 캐릭터 사망 플래그 반환 함수
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player/Enemy_Status")
	bool GetIsDead() { return bIsDead; }

	// 캐릭터 사망 애니메이션 재생
	UFUNCTION(BlueprintImplementableEvent, Category = "Player/Enemy_Animation")
	void PlayDeathAnimation();

// 무적 관련 함수/변수
protected:
	// 캐릭터가 무적상태인지(피격 후 혹은 특정 패턴) 판단
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player/Enemy_Status")
	bool bIsInvincible = false;

// 이동/회피 관련 함수/변수
protected:
	// 캐릭터의 레이어 정렬 수치, 스프라이트의 y축 값을 좌,우에 따라 더해 높은 갚을 가진 스프라이트가 앞에 보이도록 만드는 역할을 함
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player/Enemy_Status")
	float SpriteLayerSortAmount = 0.0f;

	// 캐릭터가 회피상태인지 판단
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player/Enemy_Combat_Dodge")
	bool bIsDodging = false;

	// 캐릭터가 회피 가능한 상태인지, (클타임 적용/미적용 상태)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player/Enemy_Combat_Dodge")
	bool bCanDodge = true;

	// 캐릭터의 회피 대기시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player/Enemy_Combat_Dodge")
	float DodgeCooldown = 0.6f;

	// 회피 판정 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player/Enemy_Combat_Dodge")
	float DodgeDuration = 0.3f;

	// 회피 입력 시 판단 함수
	UFUNCTION(BlueprintCallable, Category = "Player/Enemy_Combat_Dodge")
	virtual bool TryDodge(float Time);

	// 회피 실행 함수
	UFUNCTION(BlueprintCallable, Category = "Player/Enemy_Combat_Dodge")
	virtual void DodgeStart(float Time);

	// 회피 종료 함수 (타이머 콜백)
	UFUNCTION()
	virtual void DodgeEnd();

	// 회피 쿨타임 종료 함수
	virtual void ResetDodgeCooldown();
	
	// 회피 애니메이션 호출 이벤트, 없다면 무시되지만 !!반드시 부모 BP가 아닌 가장 자식인 BP에서 구현해야 합니다!!
	UFUNCTION(BlueprintImplementableEvent, Category = "Player/Enemy_Animation")
	void PlayDodgeAnimation();

	// 회피 플래그 반환
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player/Enemy_Combat_Dodge")
	bool GetIsDodging() { return bIsDodging; }

	// 캐릭터의 지면마찰력 저장 변수
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player/Enemy_Combat_Dodge")
	float SavedGroundFriction;

	// 캐릭터의 이동방향에 따른 SpriteSortAmount 적용 함수, 스프라이트 컴포넌트의 상대위치를 약간 당겨 더 앞에 보이도록 만드는 함수
	UFUNCTION(BlueprintCallable, Category = "Player/Enemy_Status")
	virtual void ApplySpriteSortAmount();

// 가드 관련 함수/변수
protected:
	// 가드 플래그
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player/Enemy_Combat_Guard")
	bool bIsGuarding = false;
	
	// 캐릭터 가드 애니메이션 재생, !!반드시 부모 BP가 아닌 가장 자식인 BP에서 구현해야 합니다!!
	UFUNCTION(BlueprintImplementableEvent, Category = "Player/Enemy_Animation")
	void PlayGuardAnimation();

	// 가드 시도 시, 가드 가능 여부 판단, 가능하면 가드 호출
	UFUNCTION(BlueprintCallable, Category = "Player/Enemy_Combat_Guard")
	virtual bool TryGuard();
	
	// 실제 가드 로직 실행 함수
	UFUNCTION(BlueprintCallable, Category = "Player/Enemy_Combat_Guard")
	virtual void GuardStart();

	// 가드 종료 함수
	UFUNCTION(BlueprintCallable, Category = "Player/Enemy_Combat_Guard")
	virtual void EndGuard();

	// 가드 상태 지속 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player/Enemy_Combat_Guard")
	float GuardDuration = 0.35f;

	// 가드 성공 여부, 추후 확장 가능한 옵션에 대해 사용 가능
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player/Enemy_Combat_Guard")
	bool bIsGuardSuccess = false;

	// 가드 성공시 넉백 강도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player/Enemy_Combat_Guard")
	float GuardKnockbackStrength = 0.0f;

	// 가드 쿨타임
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player/Enemy_Combat_Guard")
	float GuardCoolDown = 1.3f;

	// 가드 쿨타임 플래그
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player/Enemy_Combat_Guard")
	bool bCanGuard = true;

	// 가드 쿨타임 초기화 함수 
	virtual void ResetGuardCooldown();

	// 가드가 단 한번의 공격만을 막을지에 대한 플래그
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player/Enemy_Combat_Guard")
	bool bIsGuardBlockOnlyOneHit = false;

	// 가드 후 마찰력을 원래대로 돌려두는 전용 함수 
	void RestoreGuardState();

// 기타 추가 기능
protected:
	// 회피무적 타이머 관리자
	FTimerHandle DodgeTimerHandle;

	// 회피 쿨타임 관리를 위한 타이머 관리자
	FTimerHandle DodgeCooldownTimerHandle;

	// 가드 판정 타이머 관리자
	FTimerHandle GuardTimerHandle;

	// 가드 쿨타임 관리자
	FTimerHandle GuardCooldownTimerHandle;

	// 가드 반동(넉백) 물리 복구 타이머 관리자
	FTimerHandle GuardRecoilTimerHandle;

	// 해당 캐릭터가 움직일 수 있는지 확인하는 함수, 자식에서 재정의하여 사용 가능
	// 캐릭터가 행동 가능한 상태라면 true, 아니라면 false 반환합니다. 
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Player/Enemy_Status")
	virtual bool IsCharacterCanAction();


	UFUNCTION(BlueprintCallable, Category = "Player/Enemy_Status")
	virtual void ResetCombatStates();

// VFX 및 오디오
protected:
	// 다이나믹 머티리얼 인스턴스를 저장하는 변수
	UPROPERTY()
	class UMaterialInstanceDynamic* DynamicSpriteMat;

	// 피격시 발생하는 나이아가라 시스템 할당 변수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player/Enemy_VFX")
	class UNiagaraSystem* HitEffect = nullptr;

	// 가드시 발생하는 나이아가라 시스템 할당 변수
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player/Enemy_VFX")
	class UNiagaraSystem* GuardEffect = nullptr;
};

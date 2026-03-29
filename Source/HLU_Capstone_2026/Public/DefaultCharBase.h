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
	virtual void ReceiveDamage_Implementation(const FDamageData& DamageData) override;

	// IDamageable 인터페이스 구현
	virtual void OnDeath_Implementation() override;

	// IDamageable::ReceiveDamage로 연결. 외부에서 ApplyDamage 호출 시 이 함수가 자동으로 실행됨.
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	// 태그가 저장되는 변수 **** 디테일창에서 태그 바꾸면 됨!!! ****
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	FGameplayTagContainer CharacterTags;

	// 태그 (적, 플레이어) 구분을 위한 인터페이스 함수 오버라이드 
	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

// 컴포넌트 생성 -------------------
protected:
	// 체력 관리 컴포넌트 - VisibleAnywhere로 디테일 패널에서 확인 가능
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UHealthComponent* HealthComponent;

	// 공격 판정 범위 - 박스 형태의 트리거
	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* AttackBox;

// 공격 기능 함수/변수 -------------------
protected:
	// 공격이 시작될 때, 공격이 가능한지 확인하는 함수, 호출될 때 공격이 가능하면 공격, 아닐경우 공격하지 않습니다. 자식 클래스에서 재정의 가능
	UFUNCTION(BlueprintCallable, Category = "Combat")
	virtual void TryAttack();

	// 공격 실행 시 호출되는 이벤트
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void Attack();

	// 한 번 공격에 맞은 액터 목록 (중복방지)
	UPROPERTY()
	TArray<AActor*> THitActors;

	// BoxOverlap 물리효과를 QueryOnly로 전환, 일반적으로 캐릭터 ABP의 노티파이에서 함수를 호출함
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartAttackCollision();

	// BoxOverlap 물리효과를 NoCollision으로 전환, 일반덕으로 캐릭터 ABP의 노티파이에서 함수를 호출함
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndAttackCollision();

	// 공격 상태 종료
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndAttackState();

	// 공격 대상을 구분하는 함수(적/플레이어), OnAttackBoxOverlap에서 호출됩니다. 
	virtual bool CanAttackTarget(AActor* Target) const;

	// 공격 박스 Overlap 이벤트, BasePlayer 클래스 완성 후 캐스팅해서 데미지 적용 예정.
	UFUNCTION()
	void OnAttackBoxOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	
	// 공격력 리턴
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Combat")
	float GetDefaultDamage() const { return DefaultDamage; }

	// 공격력 재설정 함수
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void SetDefaultDamage(float Amount);

	// 캐릭터 기본 공격력
	UPROPERTY(EditAnywhere, Category = "Combat")
	float DefaultDamage = 1;

	// 캐릭터가 공격 도중인지 판단
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bIsAttacking = false;

	// 캐릭터가 무적상태인지(피격 후 혹은 특정 패턴) 판단
	UPROPERTY(BlueprintReadWrite, Category = "Combat")
	bool bIsInvincible = false;

// 넉백 관련 함수/변수 -------------------
protected:
	// 넉백 적용 함수
	UFUNCTION()
	void PlayKnockBack(const FDamageData& DamageData);

	// 캐릭터가 넉백중인지 아닌지 아닌지 판단
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bIsKnockBack = false;

	// 캐릭터가 넉백에 면역인지 아닌지 판단
	bool bIsKnockBackImmune = false;

	// 캐릭터의 넉백 강도
	UPROPERTY(EditAnywhere, Category = "Combat")
	float KnockbackStrength = 500.f;
};

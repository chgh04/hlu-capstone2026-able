#pragma once

#include "CoreMinimal.h"
#include "PaperZDCharacter.h"
#include "Damageable.h"
#include "DefaultCharBase.generated.h"

class UHealthComponent;
class UBoxComponent;

/**
 * 
 */
UCLASS()
class HLU_CAPSTONE_2026_API ADefaultCharBase : public APaperZDCharacter, public IDamageable
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
	// 공격 실행 시 호출되는 함수(미구현). C++에서는 빈 구현만 제공하고 블루프린트에서 override해서 사용.
	UFUNCTION(BlueprintNativeEvent, Category = "Combat")
	void Attack();

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

private:
	// 캐릭터 기본 공격력
	UPROPERTY(EditAnywhere, Category = "Combat")
	float DefaultDamage = 1;

// 넉백 관련 함수/변수 -------------------
protected:
	// 넉백 적용 함수
	UFUNCTION()
	void PlayKnockBack(const FDamageData& DamageData);

	// 캐릭터가 넉백 되는지 아닌지 판단
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bIsKnockBack = true;

	// 캐릭터의 넉백 강도
	UPROPERTY(EditAnywhere, Category = "Combat")
	float KnockbackStrength = 500.f;
};

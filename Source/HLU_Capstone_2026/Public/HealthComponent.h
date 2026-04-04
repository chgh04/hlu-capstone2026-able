#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Project_Types.h"
#include "HealthComponent.generated.h"

/*
UHealthComponent - 체력 관리 컴포넌트

역할
체력이 필요한 모든 액터에 부착해서 사용하는 컴포넌트
체력 감소, 사망 판정을 담당하며 오너 액터에게 직접 함수를 호출

사용 방법

액터 생성자에서 CreateDefaultSubobject로 생성한다.
예) HealthComponent = CreateDefaultSubobject(TEXT("HealthComponent"));

해당 액터가 IDamageable을 구현하고 있어야 체력 0 시 OnDeath가 정상 호출

확장 가능한 점
체력 회복 기능 추가 시 HealHealth(float Amount) 함수를 추가
방어력 시스템 추가 시 ReduceHealth 내부에서 데미지 계산 로직을 추가
무적 시간 추가 시 bIsInvincible 플래그와 타이머를 추가

변경 시 주의
MaxHealth는 블루프린트 디테일 패널에서 조정 가능(EditAnywhere).
적마다 체력을 다르게 주려면 상속받은 BP에서 값을 바꾸면 됨.
*/

// 델리게이트 선언 -------------------
// 체력이 변경될 때 UI갱신용 델리게이트 선언 (현재 체력, 최대 체력 전달)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHealthChangedSignature, float, CurrentHealth, float, MaxHealth);

// 사망했을 때 UI갱신용 델리게이트 선언
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDeathSignature);

// 피해를 받았을 때 피해 정보를 피해자에게 전달 (Actor별 피해 방식을 다르게 하기 위함)
// 실제 플레이어에게 주는 피해 및 각종 영향을 전달합니다. 
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTakeDamageSignature, const FDamageData&, DamageData);


UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HLU_CAPSTONE_2026_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();

protected:
    virtual void BeginPlay() override;

// 추후 UI 업데이트용 델리게이트 변수 선언  -------------------
public:
    // UI에서 이 이벤트를 바인딩할 수 있게 public 설정
    UPROPERTY(BlueprintAssignable, Category = "Damage_Events")
    FOnHealthChangedSignature OnHealthChanged;

    UPROPERTY(BlueprintAssignable, Category = "Damage_Events")
    FOnDeathSignature OnDeath;

// 체력 관련 함수 및 변수 -------------------
public:
    //ReduceHealth - 체력 감소 함수 IDamageable::ReceiveDamage_Implementation 내부에서 호출. 체력이 0이 되면 bIsDead = true 후 오너의 OnDeath 호출
    UFUNCTION(BlueprintCallable, Category = "Health")
    void ReduceHealth(const float DamageAmount);

    // 현재 체력 반환 - UI나 AI에서 참조용
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
    float GetCurrentHealth() const { return CurrentHealth; }

    // 현재 체력 회복 함수
    UFUNCTION(BlueprintCallable, Category = "Health")
    void HealHealth(float Amount);

    // 최대 체력 반환
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
    float GetMaxHealth() const { return MaxHealth; }

    // 최대 체력 재설정 함수
    UFUNCTION(BlueprintCallable, Category = "Health")
    void SetMaxHealth(float Amount);

    // 사망 여부 반환 - 사망 시 true
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
    bool IsDead() const { return bIsDead; }

private:
    // 최대 체력 - 블루프린트 디테일 패널에서 적마다 조정 가능
    UPROPERTY(EditAnywhere, Category = "Health", meta = (ClampMin = "1.0"))
    float MaxHealth = 100.f;

    // 현재 체력 - 게임 중 변경되므로 VisibleAnywhere (읽기 전용 표시)
    UPROPERTY(VisibleAnywhere, Category = "Health")
    float CurrentHealth = 0.f;

    // 사망 플래그 - true가 되면 ReduceHealth 무시됨 (중복 사망 방지)
    UPROPERTY(VisibleAnywhere, Category = "Health")
    bool bIsDead = false;
};
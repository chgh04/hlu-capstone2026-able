#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HLU_CAPSTONE_2026_API UHealthComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UHealthComponent();

    //ReduceHealth - 체력 감소 함수 IDamageable::ReceiveDamage_Implementation 내부에서 호출. 체력이 0이 되면 bIsDead = true 후 오너의 OnDeath 호출
    UFUNCTION(BlueprintCallable, Category = "Health")
    void ReduceHealth(float Amount); 

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

protected:
    virtual void BeginPlay() override;

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
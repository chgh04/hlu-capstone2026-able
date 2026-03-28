#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Damageable.generated.h"

/*
IDamageable - 데미지 처리 인터페이스

역할
데미지를 받을 수 있는 모든 액터가 공통으로 구현하는 인터페이스.
TakeDamage 호출에서 ReceiveDamage를 거쳐 HealthComponent의 ReduceHealth로 이어지는 데미지 처리 흐름의 핵심 연결 고리

사용 방법
데미지를 받아야 하는 클래스에서 IDamageable을 다중 상속받고 ReceiveDamage_Implementation, OnDeath_Implementation을 구현
예시: class ABaseEnemy : public APaperZDCharacter, public IDamageable

확장 가능한 점
데미지 타입(불, 독, 물리 등) 구분이 필요하면 float을 구조체로 변경 가능
무적 상태(Invincible) 함수를 추가 가능
피격 방향이 필요하면 FVector HitDirection 파라미터를 추가 가능

변경 시 주의
이 인터페이스를 상속받는 모든 클래스에 영향
파라미터 변경 시 BaseEnemy, BasePlayer 등을 전부 수정
*/

// 공격 정보에 대한 구조체 정의
USTRUCT(BlueprintType)
struct FDamageData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    float DamageAmount = 0.f;

    UPROPERTY(BlueprintReadWrite)
    AActor* DamageCauser = nullptr;

    // 넉백 처리를 위해 필요
    UPROPERTY(BlueprintReadWrite)
    FVector HitDirection = FVector::ZeroVector; 
};

UINTERFACE(MinimalAPI, Blueprintable)
class UDamageable : public UInterface
{
    GENERATED_BODY()
};

// 실제 인터페이스 클래스 - 이걸 상속받아서 사용
class HLU_CAPSTONE_2026_API IDamageable
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, Category = "Damage")
    void ReceiveDamage(const FDamageData& DamageData); //데미지 수신 함수 - 액터의 TakeDamage가 호출될 때 내부에서 이 함수를 호출 

    UFUNCTION(BlueprintNativeEvent, Category = "Damage")
    void OnDeath();//사망 처리 함수 - HealthComponent에서 체력이 0이 됐을 때 오너 액터에게 직접 호출
};
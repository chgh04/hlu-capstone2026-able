#pragma once

#include "CoreMinimal.h"
#include "DefaultCharBase.h"
#include "EnemyBase.generated.h"

// 전방 선언 - include 대신 사용해서 컴파일 속도 향상
class UHealthComponent;
class UBoxComponent;
class USphereComponent;

UCLASS()
class HLU_CAPSTONE_2026_API AEnemyBase : public ADefaultCharBase
{
    GENERATED_BODY()

public:
    AEnemyBase();

protected:
    virtual void BeginPlay() override;

// 인터페이스 구현 및 상속 -------------------
public:

    // 부모 클래스에서 override
    virtual void OnDeath_Implementation() override;

// 컴포넌트 생성 -------------------
protected:
    // 플레이어 감지 범위 - 구 형태의 트리거
    UPROPERTY(VisibleAnywhere, Category = "Components")
    USphereComponent* DetectionRange;

// Enemy 공격 기능 함수/변수 -------------------
protected:
    // 공격 기능 관련 함수-------------------
    // 상속받은 Attack_Implemetation 함수 구현
    virtual void Attack_Implementation() override;

    // 공격 대상을 구분하는 함수(플레이어) 구체화
    virtual bool CanAttackTarget(AActor* Target) const;

// 플레이어 감지 관련 함수/변수 -------------------
    // 플레이어 감지 함수 11111111DetectionRadius 안에 플레이어가 있으면 TargetPlayer에 저장. 범위 밖이면 nullptr로 초기화.
    UFUNCTION()
    void DetectPlayer();

    // 감지된 플레이어 저장 - 블루프린트에서 이동 로직에 활용, nullptr: 감지 안된 상태
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    AActor* TargetPlayer;

    // 플레이어 감지 반경 - 블루프린트 디테일 패널에서 적마다 조정 가능
    UPROPERTY(EditAnywhere, Category = "Combat")
    float DetectionRadius = 500.f;

private:
    // 1초 감지 타이머 핸들 - ClearTimer 호출 시 필요
    FTimerHandle DetectionTimerHandle;
};
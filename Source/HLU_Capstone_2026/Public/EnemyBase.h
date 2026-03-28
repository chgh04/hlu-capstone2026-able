#pragma once

#include "CoreMinimal.h"
#include "DefaultCharBase.h"
#include "EnemyBase.generated.h"

// 전방 선언 - include 대신 사용해서 컴파일 속도 향상
class UHealthComponent;
class UBoxComponent;
class USphereComponent;

/*
    EnemyBase - 모든 적 캐릭터의 베이스 클래스

    역할
    모든 적이 공통으로 가지는 기능(체력, 감지, 공격 판정)을 C++로 구현
    실제 이동 패턴, 공격 로직, 애니메이션은 블루프린트에서 구현

    상속 구조
    APaperZDCharacter (PaperZD 캐릭터 베이스)
    └─ AEmeyBase (이 클래스)
    └─ BP_Enemy_슬라임 등 (블루프린트에서 상속)

    사용 방법

    언리얼 에디터에서 이 클래스를 부모로 하는 블루프린트를 생성
    블루프린트에서 Attack, OnDeath 함수를 override해서 구체적인 동작을 구현
    디테일 패널에서 DetectionRadius, MaxHealth 등을 조정

    C++에서 구현한 것
    플레이어 감지 (1초 타이머)
    체력 컴포넌트 연결
    TakeDamage에서 ReduceHealth로 이어지는 흐름
    공격 박스 Overlap 감지

    블루프린트에서 구현할 것
    이동 로직 (TargetPlayer 방향으로 이동)
    Attack 함수 override (공격 애니메이션, 실제 데미지)
    OnDeath override (사망 애니메이션, 아이템 드롭)

    확장 가능한 점
    패트롤 기능 : 웨이포인트 배열 추가 후 타이머로 순환
    피격 리액션 : ReceiveDamage에서 히트 애니메이션을 재생
    드롭 아이템 : OnDeath에서 SpawnActor로 아이템을 생성
 */

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

    // 플레이어 감지 관련 함수/변수 -------------------
    // 플레이어 감지 함수 (1초마다 실행) DetectionRadius 안에 플레이어가 있으면 TargetPlayer에 저장. 범위 밖이면 nullptr로 초기화.
    UFUNCTION()
    void DetectPlayer();

    // 감지된 플레이어 저장 - 블루프린트에서 이동 로직에 활용, nullptr: 감지 안된 상태
    UPROPERTY(BlueprintReadOnly, Category = "Combat")
    APawn* TargetPlayer;

    // 플레이어 감지 반경 - 블루프린트 디테일 패널에서 적마다 조정 가능
    UPROPERTY(EditAnywhere, Category = "Combat")
    float DetectionRadius = 500.f;

private:
    // 1초 감지 타이머 핸들 - ClearTimer 호출 시 필요
    FTimerHandle DetectionTimerHandle;
};
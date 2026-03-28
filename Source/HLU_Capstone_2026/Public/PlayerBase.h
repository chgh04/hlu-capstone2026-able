#pragma once

#include "CoreMinimal.h"
#include "DefaultCharBase.h"
#include "PlayerBase.generated.h"

/**
 플레이어 캐릭터의 베이스 클래스입니다. 

 ADefaultCharBase 클래스를 상속받으며,
 HealthComponent와의 통신을 위해 IDamageable 인터페이스를 구현합니다. 
 */

UCLASS()
class HLU_CAPSTONE_2026_API APlayerBase : public ADefaultCharBase
{
	GENERATED_BODY()

public:
    APlayerBase();

protected:
    virtual void BeginPlay() override;
    
// 공격 기능 함수/변수 -------------------
    // 상속받은 Attack_Implemetation 함수 구현
    virtual void Attack_Implementation() override;

    // 공격 대상을 구분하는 함수(적) 구체화
    virtual bool CanAttackTarget(AActor* Target) const;
};

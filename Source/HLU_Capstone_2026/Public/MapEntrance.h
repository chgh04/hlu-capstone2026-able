#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "MapEntrance.generated.h"

UCLASS()
class HLU_CAPSTONE_2026_API AMapEntrance : public AActor
{
    GENERATED_BODY()

public:
    AMapEntrance();

protected:
    virtual void BeginPlay() override;

    // 컴포넌트
protected:
    // 입구 위치 표시용 - 에디터에서 위치 확인용
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UBillboardComponent* Billboard;

    // 입구 설정
public:
    // 이 입구의 고유 태그 - MapTransitionTrigger의 TargetEntranceTag와 매칭
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entrance")
    FGameplayTag EntranceTag;

    // 플레이어가 이 입구에서 스폰될 때 바라볼 방향 (1 = 오른쪽, -1 = 왼쪽)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Entrance")
    float FacingDirection = 1.f;

    // 스폰 기능
public:
    // 플레이어를 이 입구 위치로 이동시킴 - MapTransitionTrigger에서 호출
    UFUNCTION(BlueprintCallable, Category = "Entrance")
    void TeleportPlayerToEntrance(ACharacter* Player);
};
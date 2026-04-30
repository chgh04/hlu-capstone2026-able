#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GameplayTagContainer.h"
#include "PilgrimGameInstance.generated.h"

UCLASS()
class HLU_CAPSTONE_2026_API UPilgrimGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UPilgrimGameInstance();

    // 레벨 전환 데이터
public:
    // 다음 레벨 로드 후 스폰할 입구 태그 - MapTransitionTrigger에서 설정
    UPROPERTY(BlueprintReadWrite, Category = "LevelTransition")
    FGameplayTag TargetEntranceTag;

    // 현재 로드된 서브레벨 이름 - 언로드 시 사용
    UPROPERTY(BlueprintReadWrite, Category = "LevelTransition")
    FName CurrentSubLevelName;
};
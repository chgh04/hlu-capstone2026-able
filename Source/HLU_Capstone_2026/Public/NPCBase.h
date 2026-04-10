#pragma once

#include "CoreMinimal.h"
#include "InteractableBase.h"
#include "NPCBase.generated.h"

UENUM(BlueprintType)
enum class ENPCType : uint8
{
    None        UMETA(DisplayName = "None"), // 없음
    Blacksmith  UMETA(DisplayName = "Blacksmith"), // 대장장이
    Shop        UMETA(DisplayName = "Shop"), // 상점
    Story       UMETA(DisplayName = "Story") // 스토리 안내
};

UCLASS()
class HLU_CAPSTONE_2026_API ANPCBase : public AInteractableBase
{
    GENERATED_BODY()

public:
    ANPCBase();

protected:
    virtual void BeginPlay() override;

public:
    // 디테일 패널 드롭다운에서 NPC 유형 선택
    // 유형에 따라 OnInteract에서 다른 기능 수행
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    ENPCType NPCType = ENPCType::None;

    // NPC 이름 - 대화창 상단에 표시 예정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    FString NPCName = TEXT("NPC");

    // NPC 대화 내용 - 배열로 여러 줄 대화 가능
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC")
    TArray<FString> DialogueLines;

    // 현재 대화 인덱스
    UPROPERTY(BlueprintReadOnly, Category = "NPC")
    int32 CurrentDialogueIndex = 0;

    // 현재 대화 중인지 판단
    UPROPERTY(BlueprintReadOnly, Category = "NPC")
    bool bIsInDialogue = false;

public:
    // AInteractableBase의 OnInteract 구현
    virtual void OnInteract_Implementation(AActor* Interactor) override;

    // 대화 다음 줄로 넘기기 - UI에서 호출
    UFUNCTION(BlueprintCallable, Category = "NPC")
    void AdvanceDialogue();

    // 대화 초기화
    UFUNCTION(BlueprintCallable, Category = "NPC")
    void ResetDialogue();

    // 대화 시작 시 UI에 알리는 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "NPC")
    FOnInteractSignature OnDialogueStart;

protected:
    // 자식 BP에서 구현 - 대화 UI 표시
    UFUNCTION(BlueprintImplementableEvent, Category = "NPC")
    void ShowDialogueUI(const FString& Line);

    // 자식 BP에서 구현 - 대화 UI 닫기
    UFUNCTION(BlueprintImplementableEvent, Category = "NPC")
    void HideDialogueUI();
};
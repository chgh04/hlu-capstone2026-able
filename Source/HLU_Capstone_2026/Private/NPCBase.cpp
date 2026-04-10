#include "NPCBase.h"

ANPCBase::ANPCBase()
{
    PrimaryActorTick.bCanEverTick = false;
}

void ANPCBase::BeginPlay()
{
    Super::BeginPlay();
}

void ANPCBase::OnInteract_Implementation(AActor* Interactor)
{
    Super::OnInteract_Implementation(Interactor);

    if (DialogueLines.Num() == 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("NPC: [%s] 대화 내용이 없습니다."), *NPCName);
        return;
    }

    // 현재 대화 줄 UI에 표시
    ShowDialogueUI(DialogueLines[CurrentDialogueIndex]);
    UE_LOG(LogTemp, Warning, TEXT("NPC: [%s] %s"), *NPCName, *DialogueLines[CurrentDialogueIndex]);
}

void ANPCBase::AdvanceDialogue()
{
    CurrentDialogueIndex++;

    if (CurrentDialogueIndex >= DialogueLines.Num())
    {
        // 대화 끝
        ResetDialogue();
        HideDialogueUI();
        return;
    }

    ShowDialogueUI(DialogueLines[CurrentDialogueIndex]);
}

void ANPCBase::ResetDialogue()
{
    CurrentDialogueIndex = 0;
}
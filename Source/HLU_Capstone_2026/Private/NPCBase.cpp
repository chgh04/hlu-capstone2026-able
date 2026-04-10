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

    if (DialogueLines.Num() == 0) return;

    if (!bIsInDialogue)
    {
        // 대화 시작
        bIsInDialogue = true;
        CurrentDialogueIndex = 0;
        ShowDialogueUI(DialogueLines[CurrentDialogueIndex]);
    }
    else
    {
        // 이미 대화 중이면 다음 줄로
        AdvanceDialogue();
    }
}

void ANPCBase::AdvanceDialogue()
{
    CurrentDialogueIndex++;

    if (CurrentDialogueIndex >= DialogueLines.Num())
    {
        // 대화 끝
        ResetDialogue();
        HideDialogueUI();
        bIsInDialogue = false;
        return;
    }

    ShowDialogueUI(DialogueLines[CurrentDialogueIndex]);
}

void ANPCBase::ResetDialogue()
{
    CurrentDialogueIndex = 0;
}
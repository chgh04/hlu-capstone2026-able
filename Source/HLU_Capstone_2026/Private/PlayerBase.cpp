#include "PlayerBase.h"

APlayerBase::APlayerBase()
{

}

void APlayerBase::BeginPlay()
{
    Super::BeginPlay();
}

void APlayerBase::Attack_Implementation()
{
    Super::Attack_Implementation();

    UE_LOG(LogTemp, Warning, TEXT("Player is now attack"));
}

#include "InteractableBase.h"
#include "Components/SphereComponent.h"
#include "PaperSpriteComponent.h"
#include "PlayerBase.h"

AInteractableBase::AInteractableBase()
{
    PrimaryActorTick.bCanEverTick = false;

    InteractRange = CreateDefaultSubobject<USphereComponent>(TEXT("InteractRange"));
    RootComponent = InteractRange;
    InteractRange->SetSphereRadius(InteractRadius);
    InteractRange->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteractRange->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
    InteractRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    SpriteComponent = CreateDefaultSubobject<UPaperSpriteComponent>(TEXT("SpriteComponent"));
    SpriteComponent->SetupAttachment(RootComponent);
    SpriteComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    InteractEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("InteractEffect"));
    InteractEffect->SetupAttachment(RootComponent);
    InteractEffect->SetAutoActivate(false);
}

void AInteractableBase::BeginPlay()
{
    Super::BeginPlay();

    InteractRange->SetSphereRadius(InteractRadius);

    if (InteractRange)
    {
        InteractRange->OnComponentBeginOverlap.AddDynamic(this, &AInteractableBase::OnInteractRangeBeginOverlap);
        InteractRange->OnComponentEndOverlap.AddDynamic(this, &AInteractableBase::OnInteractRangeEndOverlap);
    }
}

void AInteractableBase::OnInteractRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    APlayerBase* Player = Cast<APlayerBase>(OtherActor);
    if (!Player) return;

    bPlayerInRange = true;
    ShowInteractHint();
}

void AInteractableBase::OnInteractRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APlayerBase* Player = Cast<APlayerBase>(OtherActor);
    if (!Player) return;

    bPlayerInRange = false;
    HideInteractHint();
}

void AInteractableBase::TryInteract(AActor* Interactor)
{
    if (!bPlayerInRange) return;
    OnInteract(Interactor);
}

void AInteractableBase::OnInteract_Implementation(AActor* Interactor)
{
    // 기본 구현 없음 - 자식에서 반드시 구현
    UE_LOG(LogTemp, Warning, TEXT("Interactable: OnInteract called but not implemented in %s"), *GetName());
}
#include "InteractableBase.h"
#include "Components/SphereComponent.h"
#include "PaperSpriteComponent.h"
#include "BlueprintGameplayTagLibrary.h"
#include "InteractReceiver.h"
#include "PaperFlipbookComponent.h"

AInteractableBase::AInteractableBase()
{
    PrimaryActorTick.bCanEverTick = false;

    InteractRange = CreateDefaultSubobject<USphereComponent>(TEXT("InteractRange"));
    RootComponent = InteractRange;
    InteractRange->SetSphereRadius(InteractRadius);
    InteractRange->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    InteractRange->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
    InteractRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    FlipbookComponent = CreateDefaultSubobject<UPaperFlipbookComponent>(TEXT("FlipbookComponent"));
    FlipbookComponent->SetupAttachment(RootComponent);
    FlipbookComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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

void AInteractableBase::OnInteractRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 태그로 플레이어인지 확인
    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(OtherActor);

    if (TagInterface && TagInterface->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Entity.Team.Player"))))
    {
        bPlayerInRange = true;
        ShowInteractHint();

        // 플레이어에게 자신을 등록 - F키 입력 시 HandleInteractInput이 이 목록을 순회
        if (OtherActor->Implements<UInteractReceiver>())
        {
            IInteractReceiver::Execute_RegisterNearbyInteractable(OtherActor, this);
        }
    }
}

void AInteractableBase::OnInteractRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // 태그로 플레이어인지 확인
    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(OtherActor);

    if (TagInterface && TagInterface->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Entity.Team.Player"))))
    {
        bPlayerInRange = false;
        HideInteractHint(); // 버그 수정: 기존엔 ShowInteractHint()가 잘못 호출되고 있었음

        // 플레이어에서 자신을 해제
        if (OtherActor->Implements<UInteractReceiver>())
        {
            IInteractReceiver::Execute_UnregisterNearbyInteractable(OtherActor, this);
        }
    }
}

void AInteractableBase::TryInteract_Implementation(AActor* Interactor)
{
    if (!bPlayerInRange) return;
    OnInteract(Interactor);
}

void AInteractableBase::OnInteract_Implementation(AActor* Interactor)
{
    //기본 구현 없음 - 자식에서 반드시 구현
    //AInteractableBase(부모) → ACheckpointBase(자식) 구조
    //자식이 OnInteract_Implementation을 오버라이드할 때 안에서 Super::OnInteract_Implementation()을 호출
    //이때 아래 로그 실행.
    UE_LOG(LogTemp, Warning, TEXT("Interactable: OnInteract called but not implemented in %s"), *GetName());
}
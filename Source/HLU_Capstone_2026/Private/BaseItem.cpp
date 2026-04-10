#include "BaseItem.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerBase.h"
#include "GameplayTagsModule.h"

ABaseItem::ABaseItem()
{
    PrimaryActorTick.bCanEverTick = false;

    PickupRange = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRange"));
    RootComponent = PickupRange;
    PickupRange->SetSphereRadius(80.f);
    PickupRange->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    PickupRange->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
    PickupRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // 줍기 전부터 켜져 있는 이펙트 컴포넌트
    // SetAutoActivate(true) - 게임 시작하자마자 자동으로 재생됨
    PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
    PickupEffectComponent->SetupAttachment(RootComponent);
    PickupEffectComponent->SetAutoActivate(true);
}

void ABaseItem::BeginPlay()
{
    Super::BeginPlay();

    if (PickupRange)
    {
        PickupRange->OnComponentBeginOverlap.AddDynamic(this, &ABaseItem::OnPickupRangeBeginOverlap);
        PickupRange->OnComponentEndOverlap.AddDynamic(this, &ABaseItem::OnPickupRangeEndOverlap);
    }

    // 디테일 패널에서 지정한 IdleEffect 에셋을 컴포넌트에 연결
    if (IdleEffect && PickupEffectComponent)
    {
        PickupEffectComponent->SetAsset(IdleEffect);
    }
}

void ABaseItem::OnPickupRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
    bool bFromSweep, const FHitResult& SweepResult)
{
    APlayerBase* Player = Cast<APlayerBase>(OtherActor);
    if (!Player) return;

    bPlayerInRange = true;

    FGameplayTag AutoTag = FGameplayTag::RequestGameplayTag(FName("Item.Pickup.Auto"));
    if (PickupTag == AutoTag)
    {
        ExecutePickup(Player);
        return;
    }

    // Input 방식이면 F키 프롬프트만 표시
    ShowPickupHint();
}

void ABaseItem::OnPickupRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    APlayerBase* Player = Cast<APlayerBase>(OtherActor);
    if (!Player) return;

    bPlayerInRange = false;
    HidePickupHint();
}

void ABaseItem::TryPickupByInput(AActor* Picker)
{
    if (!bPlayerInRange || bIsPickedUp) return;
    ExecutePickup(Picker);
}

void ABaseItem::ExecutePickup(AActor* Picker)
{
    if (bIsPickedUp) return;
    bIsPickedUp = true;

    UE_LOG(LogTemp, Warning, TEXT("Item: [%s] picked up!"), *ItemName);

    OnItemPickedUp.Broadcast(ItemName);

    // 콜리전 제거
    if (PickupRange)
    {
        PickupRange->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 이펙트 끄기 - 아이템이 사라진 것처럼 보임
    if (PickupEffectComponent)
    {
        PickupEffectComponent->Deactivate();
    }

    HidePickupHint();

    // DestroyDelay 후 액터 삭제
    GetWorldTimerManager().SetTimer(
        DestroyTimerHandle,
        this,
        &ABaseItem::DestroyAfterEffect,
        DestroyDelay,
        false
    );
}

void ABaseItem::DestroyAfterEffect()
{
    Destroy();
}

void ABaseItem::OnPickedUp_Implementation(AActor* Picker)
{
    ExecutePickup(Picker);
}

void ABaseItem::OnUsed_Implementation(AActor* User)
{
    UE_LOG(LogTemp, Warning, TEXT("Item: [%s] used by %s"), *ItemName, *GetNameSafe(User));
}
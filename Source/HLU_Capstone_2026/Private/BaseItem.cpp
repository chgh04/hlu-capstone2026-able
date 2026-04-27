#include "BaseItem.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameplayTagsModule.h"
#include "BlueprintGameplayTagLibrary.h"
#include "InteractReceiver.h"
#include "InventoryComponent.h"

ABaseItem::ABaseItem()
{
    PrimaryActorTick.bCanEverTick = false;

    // 습득 판정 범위 생성
    // 플레이어 전용 채널(ECC_GameTraceChannel1)과만 Overlap
    PickupRange = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRange"));
    RootComponent = PickupRange;
    PickupRange->SetSphereRadius(80.f);
    PickupRange->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    PickupRange->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
    PickupRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // 위치 알림용 나이아가라 이펙트 컴포넌트 생성
    // SetAutoActivate(true) - 게임 시작 시 자동 재생
    PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
    PickupEffectComponent->SetupAttachment(RootComponent);
    PickupEffectComponent->SetAutoActivate(true);
}

void ABaseItem::BeginPlay()
{
    Super::BeginPlay();

    // Overlap 이벤트 바인딩
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

void ABaseItem::OnPickedUp_Implementation(AActor* Picker)
{
    ExecutePickup(Picker);
}

void ABaseItem::OnUsed_Implementation(AActor* User)
{
    UE_LOG(LogTemp, Warning, TEXT("Item: [%s] used by %s"), *ItemData.ItemName, *GetNameSafe(User));
}

void ABaseItem::TryPickup_Implementation(AActor* Picker)
{
    // 범위 밖이거나 이미 습득한 경우 무시
    if (!bPlayerInRange || bIsPickedUp) return;

    ExecutePickup(Picker);
}

void ABaseItem::OnPickupRangeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // 태그로 플레이어인지 확인
    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(OtherActor);

    if (TagInterface && TagInterface->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Entity.Team.Player"))))
    {
        bPlayerInRange = true;

        // 플레이어에게 자신을 등록 - F키 입력 시 HandleInteractInput이 이 목록을 순회
        if (OtherActor->Implements<UInteractReceiver>())
        {
            IInteractReceiver::Execute_RegisterNearbyItem(OtherActor, this);
        }

        // Auto 태그: 범위 진입 즉시 습득
        FGameplayTag AutoTag = FGameplayTag::RequestGameplayTag(FName("Item.Pickup.Auto"));
        if (PickupTag == AutoTag)
        {
            ExecutePickup(OtherActor);
            return;
        }

        // Input 태그: F키 프롬프트만 표시하고 대기
        ShowPickupHint();
    }
}

void ABaseItem::OnPickupRangeEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    // 태그로 플레이어인지 확인
    IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(OtherActor);

    if (TagInterface && TagInterface->HasMatchingGameplayTag(FGameplayTag::RequestGameplayTag(FName("Entity.Team.Player"))))
    {
        bPlayerInRange = false;
        HidePickupHint();

        // 플레이어에서 자신을 해제
        if (OtherActor->Implements<UInteractReceiver>())
        {
            IInteractReceiver::Execute_UnregisterNearbyItem(OtherActor, this);
        }
    }
}

void ABaseItem::ExecutePickup(AActor* Picker)
{
    // 중복 습득 방지
    if (bIsPickedUp) return;
    bIsPickedUp = true;

    UE_LOG(LogTemp, Warning, TEXT("Item: [%s] picked up! Type: %s"), *ItemData.ItemName, *UEnum::GetValueAsString(ItemData.ItemType));

    // 인벤토리 완성 후 여기에 바인딩
    OnItemPickedUp.Broadcast(ItemData);

    // 플레이어 인벤토리에 직접 추가
    // IInteractReceiver를 구현한 액터(플레이어)에서 InventoryComponent를 찾아 AddItem 호출
    if (Picker)
    {
        UInventoryComponent* Inventory = Picker->FindComponentByClass<UInventoryComponent>();
        if (Inventory)
        {
            Inventory->AddItem(ItemData);
        }
    }

    // 획득 이펙트 재생
    if (PickupBurstEffect)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            PickupBurstEffect,
            GetActorLocation(),
            GetActorRotation()
        );
    }

    // 콜리전 제거 - 중복 습득 방지
    if (PickupRange)
    {
        PickupRange->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }

    // 이펙트 끄기
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


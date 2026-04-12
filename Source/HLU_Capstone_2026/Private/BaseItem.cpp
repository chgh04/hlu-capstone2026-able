#include "BaseItem.h"
#include "Components/SphereComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerBase.h"
#include "GameplayTagsModule.h"

ABaseItem::ABaseItem()
{
    PrimaryActorTick.bCanEverTick = false;

    // 습득 판정 범위 생성
    // 플레이어 전용 채널(ECC_GameTraceChannel1)과만 Overlap - EnemyBase의 DetectionRange와 동일한 방식
    PickupRange = CreateDefaultSubobject<USphereComponent>(TEXT("PickupRange"));
    RootComponent = PickupRange;
    PickupRange->SetSphereRadius(80.f);
    PickupRange->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    PickupRange->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel1, ECollisionResponse::ECR_Overlap);
    PickupRange->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

    // 위치 알림용 나이아가라 이펙트 컴포넌트 생성
    // SetAutoActivate(true) - 게임 시작 시 자동 재생되어 아이템 위치를 플레이어에게 알림
    // 습득 시 ExecutePickup에서 Deactivate() 호출로 꺼짐
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
    // 컴포넌트 디테일 패널에서 직접 지정한 경우 이 코드는 무시됨
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

    // 태그 체크와 관계없이 무조건 범위 진입 기록
    // 태그 비교가 실패해도 bPlayerInRange가 false로 남는 버그 방지
    bPlayerInRange = true;

    // Auto 태그: 범위 진입 즉시 습득
    FGameplayTag AutoTag = FGameplayTag::RequestGameplayTag(FName("Item.Pickup.Auto"));
    if (PickupTag == AutoTag)
    {
        ExecutePickup(Player);
        return;
    }

    // Input 태그: F키 프롬프트만 표시하고 대기
    // bPlayerInRange가 true이므로 F키 입력 시 TryPickupByInput에서 처리
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
    // 범위 밖이거나 이미 습득한 경우 무시
    if (!bPlayerInRange || bIsPickedUp) return;

    ExecutePickup(Picker);
}

void ABaseItem::ExecutePickup(AActor* Picker)
{
    // 중복 습득 방지 - 이미 습득됐으면 즉시 리턴
    if (bIsPickedUp) return;
    bIsPickedUp = true;

    UE_LOG(LogTemp, Warning, TEXT("Item: [%s] picked up! Type: %s"),
        *ItemData.ItemName,
        *UEnum::GetValueAsString(ItemData.ItemType));

    // UI 델리게이트 브로드캐스트 - 인벤토리 완성 후 여기에 바인딩
    OnItemPickedUp.Broadcast(ItemData);

    // 콜리전 제거 - 이펙트 재생 중 중복 습득 방지
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
    // 이펙트가 완전히 꺼진 뒤 액터를 삭제하기 위한 딜레이
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
    UE_LOG(LogTemp, Warning, TEXT("Item: [%s] used by %s"), *ItemData.ItemName, *GetNameSafe(User));
}
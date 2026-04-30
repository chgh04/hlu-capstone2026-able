#include "MapEntrance.h"
#include "Components/BillboardComponent.h"
#include "GameFramework/Character.h"

AMapEntrance::AMapEntrance()
{
    PrimaryActorTick.bCanEverTick = false;

    // 에디터에서 입구 위치 확인용 빌보드 - 게임에서는 보이지 않음
    Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));
    RootComponent = Billboard;
}

void AMapEntrance::BeginPlay()
{
    Super::BeginPlay();
}

void AMapEntrance::TeleportPlayerToEntrance(ACharacter* Player)
{
    if (!Player) return;

    // 플레이어를 입구 위치로 이동
    FVector EntranceLocation = GetActorLocation();
    Player->SetActorLocation(EntranceLocation);

    UE_LOG(LogTemp, Warning, TEXT("MapEntrance: Player teleported to %s"), *EntranceLocation.ToString());
}
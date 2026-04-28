#include "SaveLoadComponent.h"
#include "PlayerBase.h"
#include "HealthComponent.h"
#include "InventoryComponent.h"
#include "CheckpointBase.h"
#include "PilgrimSaveGame.h"
#include "Kismet/GameplayStatics.h"

USaveLoadComponent::USaveLoadComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void USaveLoadComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USaveLoadComponent::SaveGame(FVector CheckpointLocation)
{
    // 컴포넌트→오너 캐스팅: 직접 소유 관계이므로 하드캐스팅 규칙 예외
    APlayerBase* Player = Cast<APlayerBase>(GetOwner());
    if (!Player) return;

    UHealthComponent* HC = Player->FindComponentByClass<UHealthComponent>();
    UInventoryComponent* IC = Player->FindComponentByClass<UInventoryComponent>();

    UPilgrimSaveGame* SaveData = Cast<UPilgrimSaveGame>(
        UGameplayStatics::CreateSaveGameObject(UPilgrimSaveGame::StaticClass())
    );
    if (!SaveData) return;

    // 플레이어 위치 - Z+200 오프셋으로 바닥 관통 방지
    FVector SaveLocation = CheckpointLocation;
    SaveLocation.Z += 200.f;
    SaveData->PlayerRespawnLocation = SaveLocation;

    // 체력/포션
    SaveData->PlayerCurrentHealth = HC ? HC->GetCurrentHealth() : 100.f;
    SaveData->PlayerCurrentPotionCount = Player->GetCurrentPotionCount();

    // 체크포인트 활성화 목록 저장 (버그 수정)
    SaveData->ActivatedCheckpointNames = ActivatedCheckpointNames;

    // 인벤토리
    if (IC)
    {
        SaveData->OwnedItemCodes = IC->GetOwnedItemCodes();
        SaveData->AllItems = IC->GetAllItems();
        SaveData->RelicSlots = IC->GetRelicSlots();
        SaveData->ActiveRelicSlotCount = IC->GetActiveRelicSlotCount();
    }

    // 마지막 체크포인트 위치 캐싱 - SaveCurrentState에서 사용
    LastCheckpointLocation = CheckpointLocation;

    UGameplayStatics::SaveGameToSlot(SaveData, UPilgrimSaveGame::SaveSlotName, UPilgrimSaveGame::SaveUserIndex);

    // 세이브 완료 알림 - UI에서 바인딩하여 저장 아이콘 표시 등에 활용
    OnGameSaved.Broadcast();

    UE_LOG(LogTemp, Warning, TEXT("SaveLoad: Saved at %s, Checkpoints: %d"),
        *CheckpointLocation.ToString(), ActivatedCheckpointNames.Num());
}

void USaveLoadComponent::LoadGame()
{
    if (!UGameplayStatics::DoesSaveGameExist(UPilgrimSaveGame::SaveSlotName, UPilgrimSaveGame::SaveUserIndex))
    {
        UE_LOG(LogTemp, Warning, TEXT("SaveLoad: No save file found"));
        return;
    }

    UPilgrimSaveGame* SaveData = Cast<UPilgrimSaveGame>(
        UGameplayStatics::LoadGameFromSlot(UPilgrimSaveGame::SaveSlotName, UPilgrimSaveGame::SaveUserIndex)
    );
    if (!SaveData) return;

    APlayerBase* Player = Cast<APlayerBase>(GetOwner());
    if (!Player) return;

    UHealthComponent* HC = Player->FindComponentByClass<UHealthComponent>();
    UInventoryComponent* IC = Player->FindComponentByClass<UInventoryComponent>();

    // 플레이어 상태 복원
    Player->SetActorLocation(SaveData->PlayerRespawnLocation);
    Player->SetCurrentRespawnLocation(SaveData->PlayerRespawnLocation);
    Player->SetCurrentPotionCount(SaveData->PlayerCurrentPotionCount);

    if (HC)
    {
        float Delta = SaveData->PlayerCurrentHealth - HC->GetCurrentHealth();
        HC->HealHealth(Delta);
    }

    if (IC)
    {
        IC->LoadFromSaveData(
            SaveData->OwnedItemCodes,
            SaveData->AllItems,
            SaveData->RelicSlots,
            SaveData->ActiveRelicSlotCount
        );
    }

    // 체크포인트 활성화 목록 복원
    ActivatedCheckpointNames = SaveData->ActivatedCheckpointNames;

    // LoadGame은 BeginPlay 시 1회만 호출 → GetAllActorsOfClass 허용
    TArray<AActor*> AllCheckpoints;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACheckpointBase::StaticClass(), AllCheckpoints);
    for (AActor* Actor : AllCheckpoints)
    {
        ACheckpointBase* Checkpoint = Cast<ACheckpointBase>(Actor);
        if (Checkpoint && ActivatedCheckpointNames.Contains(Checkpoint->GetCheckpointName()))
        {
            Checkpoint->bIsActivated = true;
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("SaveLoad: Loaded. Respawn: %s, Items: %d, Checkpoints: %d"),
        *SaveData->PlayerRespawnLocation.ToString(),
        IC ? IC->GetAllItems().Num() : -1,
        ActivatedCheckpointNames.Num());
}

void USaveLoadComponent::DeleteSaveGame()
{
    UGameplayStatics::DeleteGameInSlot(UPilgrimSaveGame::SaveSlotName, UPilgrimSaveGame::SaveUserIndex);
    ActivatedCheckpointNames.Empty();
    LastCheckpointLocation = FVector::ZeroVector;  

    UE_LOG(LogTemp, Warning, TEXT("SaveLoad: Save file deleted"));
}

void USaveLoadComponent::RegisterActivatedCheckpoint(const FString& CheckpointName)
{
    if (!ActivatedCheckpointNames.Contains(CheckpointName))
    {
        ActivatedCheckpointNames.Add(CheckpointName);
        UE_LOG(LogTemp, Warning, TEXT("SaveLoad: Checkpoint registered - %s"), *CheckpointName);
    }
}

bool USaveLoadComponent::IsCheckpointActivated(const FString& CheckpointName) const
{
    return ActivatedCheckpointNames.Contains(CheckpointName);
}

void USaveLoadComponent::SaveCurrentState()
{
    // 아직 한 번도 체크포인트에서 저장한 적 없으면 무시
    if (LastCheckpointLocation == FVector::ZeroVector) return;

    SaveGame(LastCheckpointLocation);

    UE_LOG(LogTemp, Warning, TEXT("SaveLoad: Auto-saved after item pickup"));
}
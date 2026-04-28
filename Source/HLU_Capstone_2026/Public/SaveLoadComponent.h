#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SaveLoadComponent.generated.h"

// 세이브 완료 시 UI에 알리기 위한 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGameSavedSignature);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class HLU_CAPSTONE_2026_API USaveLoadComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    USaveLoadComponent();

protected:
    virtual void BeginPlay() override;

    // 세이브/로드
public:
    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    void SaveGame(FVector CheckpointLocation);

    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    void LoadGame();

    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    void DeleteSaveGame();

    // 세이브 완료 시 브로드캐스트 - UI에서 저장 아이콘 표시 등에 사용
    UPROPERTY(BlueprintAssignable, Category = "SaveLoad")
    FOnGameSavedSignature OnGameSaved;

    // 체크포인트 활성화 관리
public:
    // CheckpointBase::ActivateCheckpoint에서 호출
    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    void RegisterActivatedCheckpoint(const FString& CheckpointName);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "SaveLoad")
    bool IsCheckpointActivated(const FString& CheckpointName) const;

    // 아이템 습득 시 호출 - 마지막 체크포인트 위치 기준으로 자동저장
    UFUNCTION(BlueprintCallable, Category = "SaveLoad")
    void SaveCurrentState();

private:
    UPROPERTY()
    TArray<FString> ActivatedCheckpointNames;

    // 마지막으로 저장된 체크포인트 위치 - 아이템 습득 시 자동저장에 사용
    FVector LastCheckpointLocation = FVector::ZeroVector;
};
// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GhostActor.generated.h"

/*
플레이어 회피 시 소환되는 잔상 액터입니다. 굳이 신경 안써도 됨
*/

UCLASS()
class HLU_CAPSTONE_2026_API AGhostActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AGhostActor();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, Category = "Ghost")
	class UPaperSpriteComponent* GhostSpriteComp;

	UPROPERTY()
	class UMaterialInstanceDynamic* GhostMat;

	UPROPERTY(EditDefaultsOnly, Category = "Ghost")
	float CurrentOpacity = 0.5f;

public:	
	void InitGhost(class UPaperSprite* InSprite, FVector InScale);
};

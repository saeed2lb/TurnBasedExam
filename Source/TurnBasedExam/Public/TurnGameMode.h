#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "TurnGameMode.generated.h"

UENUM(BlueprintType)
enum class EPlayerTurn : uint8
{
    Human UMETA(DisplayName = "Human Player"),
    AI    UMETA(DisplayName = "AI Player")
};

UCLASS()
class TURNBASEDEXAM_API ATurnGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    ATurnGameMode();

    // --- Game State & Turn Tracking ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn System")
    EPlayerTurn CurrentPlayerTurn;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Game State")
    bool bIsPhase4;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Turn System")
    bool bIsTurnTransitioning;

    // --- AI Deployment State ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Logic")
    bool bAIHasPlacedFirstUnit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Logic")
    bool bAIHasPlacedSecondUnit;

    // --- Objective Tracking ---
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tower Logic")
    int32 HumanTowerHoldTurns;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tower Logic")
    int32 AITowerHoldTurns;

    // --- Class References ---
    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    TSubclassOf<class AGameUnit> AISniperClass;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    TSubclassOf<class AGameUnit> AIBrawlerClass;

    // --- Core Flow Functions ---
    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void StartPlacementPhase();

    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void ExecuteAITurn();

    UFUNCTION(BlueprintCallable, Category = "Game Flow")
    void SwitchTurn();

    UFUNCTION()
    void RestartMatch();

    void DelaySwitchTurn(float DelayTime);

private:
    FTimerHandle TurnTransitionTimerHandle;
    FTimerHandle CoinFlipTimerHandle;

    void ResolveCoinFlip();
};
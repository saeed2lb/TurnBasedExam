#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainPlayerController.generated.h"

UCLASS()
class TURNBASEDEXAM_API AMainPlayerController : public APlayerController
{
    GENERATED_BODY()

public:
    AMainPlayerController();

    // --- Spawning Configuration ---
    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    TSubclassOf<class AGameUnit> HumanSniperClass;

    UPROPERTY(EditDefaultsOnly, Category = "Spawning")
    TSubclassOf<class AGameUnit> HumanBrawlerClass;

    // --- Deployment State ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Deployment")
    bool bHasPlacedFirstUnit;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Deployment")
    bool bHasPlacedSecondUnit;

    // --- Combat State ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
    class AGameUnit* SelectedUnit;

    // --- Input Handling ---
    UFUNCTION(BlueprintCallable, Category = "Input")
    void OnGridClicked();

protected:
    virtual void SetupInputComponent() override;
};
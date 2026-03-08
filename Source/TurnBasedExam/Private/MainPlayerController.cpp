#include "MainPlayerController.h"
#include "TurnGameMode.h"
#include "GridTile.h"
#include "GameUnit.h"
#include "GridManager.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

AMainPlayerController::AMainPlayerController()
{
    bShowMouseCursor = true;
    bEnableClickEvents = true;
    bEnableMouseOverEvents = true;
    bHasPlacedFirstUnit = false;
    bHasPlacedSecondUnit = false;
    SelectedUnit = nullptr;
}

void AMainPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindKey(EKeys::LeftMouseButton, IE_Pressed, this, &AMainPlayerController::OnGridClicked);
}

void AMainPlayerController::OnGridClicked()
{
    ATurnGameMode* GameMode = Cast<ATurnGameMode>(GetWorld()->GetAuthGameMode());
    if (!GameMode) return;

    FHitResult HitResult;
    GetHitResultUnderCursor(ECC_Visibility, false, HitResult);

    // ================
    // COMBAT LOGIC
    // ================
    if (bHasPlacedSecondUnit && GameMode->bAIHasPlacedSecondUnit)
    {
        // Turn validation and transition locks
        if (GameMode->CurrentPlayerTurn != EPlayerTurn::Human)
        {
            if (GameMode->bIsTurnTransitioning) return;

            if (GameMode->CurrentPlayerTurn != EPlayerTurn::Human)
            {
                UE_LOG(LogTemp, Warning, TEXT("Wait your turn!"));
                return;
            }

            UE_LOG(LogTemp, Warning, TEXT("Wait your turn!"));
            return;
        }

        if (HitResult.bBlockingHit)
        {
            // 1. Evaluate Unit Interactions
            AGameUnit* ClickedUnit = Cast<AGameUnit>(HitResult.GetActor());
            if (ClickedUnit)
            {
                // Select friendly unit
                if (ClickedUnit->PlayerID == "HP")
                {
                    SelectedUnit = ClickedUnit;
                    UE_LOG(LogTemp, Warning, TEXT("Selected Human Unit: %s"), *SelectedUnit->UnitID);
                }
                // Target enemy unit
                else
                {
                    if (SelectedUnit)
                    {
                        SelectedUnit->PerformAttack(ClickedUnit);

                        if (SelectedUnit->bHasAttackedThisTurn)
                        {
                            SelectedUnit = nullptr;
                            GameMode->DelaySwitchTurn(1.0f);
                        }
                        else
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Attack failed! Target out of range or uphill."));
                            SelectedUnit = nullptr;
                        }
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("Select one of your units first before attacking!"));
                    }
                }
                return;
            }

            // 2. Evaluate Movement Execution
            AGridTile* ClickedTile = Cast<AGridTile>(HitResult.GetActor());
            if (ClickedTile && SelectedUnit)
            {
                if (ClickedTile == SelectedUnit->CurrentTile) return;

                if (SelectedUnit->bHasMovedThisTurn)
                {
                    UE_LOG(LogTemp, Warning, TEXT("This unit has already moved this turn!"));
                    return;
                }

                AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
                if (GridManager)
                {
                    TArray<AGridTile*> Path = GridManager->FindPath(SelectedUnit->CurrentTile, ClickedTile);

                    if (Path.Num() > 0)
                    {
                        int32 TotalPathCost = Path.Last()->GCost;
                        int32 MaxAllowedCost = SelectedUnit->MovementRange * 10;

                        if (TotalPathCost <= MaxAllowedCost)
                        {
                            FString StartCoords = FString::Printf(TEXT("(%d,%d)"), SelectedUnit->CurrentTile->GridX, SelectedUnit->CurrentTile->GridY);
                            FString EndCoords = FString::Printf(TEXT("(%d,%d)"), ClickedTile->GridX, ClickedTile->GridY);

                            UE_LOG(LogTemp, Warning, TEXT("%s: %s %s -> %s"), *SelectedUnit->PlayerID, *SelectedUnit->UnitID, *StartCoords, *EndCoords);

                            SelectedUnit->MoveAlongPath(Path);
                            SelectedUnit = nullptr;

                            GameMode->DelaySwitchTurn(3.5f);
                        }
                        else
                        {
                            UE_LOG(LogTemp, Warning, TEXT("Target is too far! Cost: %d, Max Stamina: %d"), TotalPathCost, MaxAllowedCost);
                        }
                    }
                    else
                    {
                        UE_LOG(LogTemp, Warning, TEXT("No valid path exists!"));
                    }
                }
                return;
            }
        }
        return;
    }

    // =======================
    // DEPLOYMENT LOGIC
    // =======================
    if (GameMode->CurrentPlayerTurn != EPlayerTurn::Human) return;

    if (HitResult.bBlockingHit)
    {
        AGridTile* ClickedTile = Cast<AGridTile>(HitResult.GetActor());
        if (ClickedTile)
        {
            if (ClickedTile->GridY <= 2 && !ClickedTile->bIsOccupied && ClickedTile->bIsWalkable)
            {
                FVector UnitSpawnLocation = ClickedTile->GetActorLocation() + FVector(0, 0, 100.0f);
                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

                if (!bHasPlacedFirstUnit && HumanSniperClass)
                {
                    AGameUnit* SpawnedSniper = GetWorld()->SpawnActor<AGameUnit>(HumanSniperClass, UnitSpawnLocation, FRotator::ZeroRotator, SpawnParams);
                    if (SpawnedSniper)
                    {
                        SpawnedSniper->CurrentTile = ClickedTile;
                        SpawnedSniper->PlayerID = "HP";
                        SpawnedSniper->OwnerType = EUnitOwner::Human;
                    }
                    bHasPlacedFirstUnit = true;
                    ClickedTile->bIsOccupied = true;
                    GameMode->SwitchTurn();
                }
                else if (bHasPlacedFirstUnit && HumanBrawlerClass)
                {
                    AGameUnit* SpawnedBrawler = GetWorld()->SpawnActor<AGameUnit>(HumanBrawlerClass, UnitSpawnLocation, FRotator::ZeroRotator, SpawnParams);
                    if (SpawnedBrawler)
                    {
                        SpawnedBrawler->CurrentTile = ClickedTile;
                        SpawnedBrawler->PlayerID = "HP";
                        SpawnedBrawler->OwnerType = EUnitOwner::Human;
                    }
                    bHasPlacedSecondUnit = true;
                    ClickedTile->bIsOccupied = true;
                    GameMode->SwitchTurn();
                }
            }
        }
    }
}
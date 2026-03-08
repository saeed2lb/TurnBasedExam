#include "TurnGameMode.h"
#include "MainPlayerController.h"
#include "GridManager.h"
#include "GridTile.h"
#include "GameUnit.h"
#include "GameTower.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Algo/RandomShuffle.h"

ATurnGameMode::ATurnGameMode()
{
    bAIHasPlacedFirstUnit = false;
    bAIHasPlacedSecondUnit = false;
    bIsPhase4 = false;
    CurrentPlayerTurn = EPlayerTurn::Human;
    bIsTurnTransitioning = false;
    HumanTowerHoldTurns = 0;
    AITowerHoldTurns = 0;
}

void ATurnGameMode::StartPlacementPhase()
{
    GetWorldTimerManager().SetTimer(CoinFlipTimerHandle, this, &ATurnGameMode::ResolveCoinFlip, 0.5f, false);
}

void ATurnGameMode::ResolveCoinFlip()
{
    bool bAIGoesFirst = FMath::RandBool();

    if (bAIGoesFirst)
    {
        UE_LOG(LogTemp, Warning, TEXT("COIN FLIP: AI Goes First!"));
        CurrentPlayerTurn = EPlayerTurn::AI;
        ExecuteAITurn();
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("COIN FLIP: Human Goes First!"));
        CurrentPlayerTurn = EPlayerTurn::Human;
    }
}

void ATurnGameMode::SwitchTurn()
{
    AMainPlayerController* PC = Cast<AMainPlayerController>(GetWorld()->GetFirstPlayerController());

    bIsTurnTransitioning = false;

    // Phase Transition: Deployment to Combat
    if (!bIsPhase4 && PC && PC->bHasPlacedSecondUnit && bAIHasPlacedSecondUnit)
    {
        bIsPhase4 = true;
        CurrentPlayerTurn = EPlayerTurn::Human;

        TArray<AActor*> FoundUnits;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGameUnit::StaticClass(), FoundUnits);
        for (AActor* Actor : FoundUnits)
        {
            if (AGameUnit* Unit = Cast<AGameUnit>(Actor))
            {
                Unit->ResetActions();
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("===================================="));
        UE_LOG(LogTemp, Warning, TEXT("DEPLOYMENT COMPLETE! ENTERING PHASE 4."));
        UE_LOG(LogTemp, Warning, TEXT("===================================="));
        return;
    }

    // Combat Phase Win Condition Checks
    if (bIsPhase4)
    {
        TArray<AActor*> FoundUnits;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGameUnit::StaticClass(), FoundUnits);

        bool bHumanAlive = false;
        bool bAIAlive = false;

        for (AActor* Actor : FoundUnits)
        {
            if (AGameUnit* Unit = Cast<AGameUnit>(Actor))
            {
                if (Unit->PlayerID == "HP") bHumanAlive = true;
                if (Unit->PlayerID == "AI") bAIAlive = true;
            }
        }

        // Annihilation Checks
        if (!bAIAlive)
        {
            UE_LOG(LogTemp, Warning, TEXT("===================================="));
            UE_LOG(LogTemp, Warning, TEXT("HUMAN WINS! All AI units destroyed."));
            UE_LOG(LogTemp, Warning, TEXT("===================================="));

            FTimerHandle RestartTimer;
            GetWorldTimerManager().SetTimer(RestartTimer, this, &ATurnGameMode::RestartMatch, 5.0f, false);
            return;
        }
        if (!bHumanAlive)
        {
            UE_LOG(LogTemp, Warning, TEXT("===================================="));
            UE_LOG(LogTemp, Warning, TEXT("AI WINS! All Human units destroyed."));
            UE_LOG(LogTemp, Warning, TEXT("===================================="));

            FTimerHandle RestartTimer;
            GetWorldTimerManager().SetTimer(RestartTimer, this, &ATurnGameMode::RestartMatch, 5.0f, false);
            return;
        }

        // Objective Domination Checks
        TArray<AActor*> FoundTowers;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGameTower::StaticClass(), FoundTowers);

        TArray<AActor*> FoundAllUnits;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGameUnit::StaticClass(), FoundAllUnits);

        TArray<AGameUnit*> AllUnitsCast;
        for (AActor* Actor : FoundAllUnits)
        {
            if (AGameUnit* Unit = Cast<AGameUnit>(Actor)) AllUnitsCast.Add(Unit);
        }

        int32 HumanTowers = 0;
        int32 AITowers = 0;

        for (AActor* Actor : FoundTowers)
        {
            if (AGameTower* Tower = Cast<AGameTower>(Actor))
            {
                Tower->EvaluateTowerState(AllUnitsCast);

                if (Tower->CurrentOwner == ETowerOwner::Human) HumanTowers++;
                if (Tower->CurrentOwner == ETowerOwner::AI) AITowers++;
            }
        }

        if (HumanTowers >= 2)
        {
            HumanTowerHoldTurns++;
            AITowerHoldTurns = 0;

            if (HumanTowerHoldTurns >= 2)
            {
                UE_LOG(LogTemp, Warning, TEXT("===================================="));
                UE_LOG(LogTemp, Warning, TEXT("HUMAN WINS BY TOWER DOMINATION!"));
                UE_LOG(LogTemp, Warning, TEXT("===================================="));

                FTimerHandle RestartTimer;
                GetWorldTimerManager().SetTimer(RestartTimer, this, &ATurnGameMode::RestartMatch, 5.0f, false);
                return;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Human controls %d towers! Hold for 1 more turn to win!"), HumanTowers);
            }
        }
        else if (AITowers >= 2)
        {
            AITowerHoldTurns++;
            HumanTowerHoldTurns = 0;

            if (AITowerHoldTurns >= 2)
            {
                UE_LOG(LogTemp, Warning, TEXT("===================================="));
                UE_LOG(LogTemp, Warning, TEXT("AI WINS BY TOWER DOMINATION!"));
                UE_LOG(LogTemp, Warning, TEXT("===================================="));

                FTimerHandle RestartTimer;
                GetWorldTimerManager().SetTimer(RestartTimer, this, &ATurnGameMode::RestartMatch, 5.0f, false);
                return;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("AI controls %d towers! Hold for 1 more turn to win!"), AITowers);
            }
        }
        else
        {
            HumanTowerHoldTurns = 0;
            AITowerHoldTurns = 0;
        }
    }

    // Standard Turn Handoff
    if (CurrentPlayerTurn == EPlayerTurn::Human)
    {
        CurrentPlayerTurn = EPlayerTurn::AI;
        UE_LOG(LogTemp, Warning, TEXT("Passing turn to AI..."));

        FTimerHandle TimerHandle_AITurn;
        GetWorldTimerManager().SetTimer(TimerHandle_AITurn, this, &ATurnGameMode::ExecuteAITurn, 1.0f, false);
    }
    else
    {
        CurrentPlayerTurn = EPlayerTurn::Human;
        UE_LOG(LogTemp, Warning, TEXT("It is now the Human's turn!"));
    }

    // Refresh Action Points
    TArray<AActor*> FoundUnits;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGameUnit::StaticClass(), FoundUnits);
    for (AActor* Actor : FoundUnits)
    {
        if (AGameUnit* Unit = Cast<AGameUnit>(Actor))
        {
            if (Unit->PlayerID == (CurrentPlayerTurn == EPlayerTurn::Human ? "HP" : "AI"))
            {
                Unit->ResetActions();
            }
        }
    }
}

void ATurnGameMode::ExecuteAITurn()
{
    if (bIsPhase4)
    {
        UE_LOG(LogTemp, Warning, TEXT("AI is thinking..."));

        AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
        if (!GridManager) return;

        TArray<AActor*> FoundUnits;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGameUnit::StaticClass(), FoundUnits);

        TArray<AActor*> FoundTowers;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGameTower::StaticClass(), FoundTowers);

        TArray<AGameUnit*> AIUnits;
        TArray<AGameUnit*> HumanUnits;

        for (AActor* Actor : FoundUnits)
        {
            if (AGameUnit* Unit = Cast<AGameUnit>(Actor))
            {
                if (Unit->PlayerID == "AI") AIUnits.Add(Unit);
                else if (Unit->PlayerID == "HP") HumanUnits.Add(Unit);
            }
        }

        bool bActionTaken = false;
        TArray<AActor*> AllTiles;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGridTile::StaticClass(), AllTiles);

        // Core AI Decision Loop
        for (AGameUnit* ActiveAI : AIUnits)
        {
            if (ActiveAI->bHasAttackedThisTurn || ActiveAI->bHasMovedThisTurn) continue;

            AGridTile* TargetTile = nullptr;
            AGameUnit* HumanToAttack = nullptr;
            int32 MinDist = 9999;
            bool bCanAttack = false;

            // Evaluate Human Targets
            for (AGameUnit* H : HumanUnits)
            {
                int32 d = FMath::Abs(ActiveAI->CurrentTile->GridX - H->CurrentTile->GridX) +
                    FMath::Abs(ActiveAI->CurrentTile->GridY - H->CurrentTile->GridY);
                if (d < MinDist)
                {
                    MinDist = d;
                    TargetTile = H->CurrentTile;

                    if (MinDist <= ActiveAI->AttackRange && TargetTile->GridZ <= ActiveAI->CurrentTile->GridZ)
                    {
                        bCanAttack = true;
                        HumanToAttack = H;
                    }
                    else
                    {
                        bCanAttack = false;
                    }
                }
            }

            // Evaluate Objective Targets (Capture Zones)
            for (AActor* ActorTower : FoundTowers)
            {
                if (AGameTower* T = Cast<AGameTower>(ActorTower))
                {
                    if (T->CurrentOwner != ETowerOwner::AI)
                    {
                        AGridTile* BestCaptureTile = nullptr;
                        int32 BestDistToCaptureZone = 9999;

                        for (AActor* ActorTile : AllTiles)
                        {
                            if (AGridTile* Tile = Cast<AGridTile>(ActorTile))
                            {
                                if (Tile->bIsWalkable && !Tile->bIsOccupied)
                                {
                                    int32 DistToTower = FMath::Abs(Tile->GridX - T->GridX) + FMath::Abs(Tile->GridY - T->GridY);
                                    if (DistToTower <= 2)
                                    {
                                        int32 DistToAI = FMath::Abs(ActiveAI->CurrentTile->GridX - Tile->GridX) + FMath::Abs(ActiveAI->CurrentTile->GridY - Tile->GridY);
                                        if (DistToAI < BestDistToCaptureZone)
                                        {
                                            BestDistToCaptureZone = DistToAI;
                                            BestCaptureTile = Tile;
                                        }
                                    }
                                }
                            }
                        }

                        if (BestCaptureTile && BestDistToCaptureZone < MinDist)
                        {
                            MinDist = BestDistToCaptureZone;
                            TargetTile = BestCaptureTile;
                            bCanAttack = false;
                        }
                    }
                }
            }

            // Execute Optimal Action
            if (TargetTile)
            {
                if (bCanAttack && HumanToAttack)
                {
                    ActiveAI->PerformAttack(HumanToAttack);
                    bActionTaken = true;
                }
                else if (!ActiveAI->bHasMovedThisTurn)
                {
                    bool bOldWalkable = TargetTile->bIsWalkable;
                    bool bOldOccupied = TargetTile->bIsOccupied;

                    TargetTile->bIsWalkable = true;
                    TargetTile->bIsOccupied = false;

                    TArray<AGridTile*> Path = GridManager->FindPath(ActiveAI->CurrentTile, TargetTile);

                    TargetTile->bIsWalkable = bOldWalkable;
                    TargetTile->bIsOccupied = bOldOccupied;

                    if (Path.Num() > 0)
                    {
                        TArray<AGridTile*> ValidPath;
                        int32 MaxCost = ActiveAI->MovementRange * 10;
                        for (AGridTile* Tile : Path)
                        {
                            if (HumanToAttack && Tile == HumanToAttack->CurrentTile) break;
                            if (Tile->GCost <= MaxCost) ValidPath.Add(Tile);
                        }

                        if (ValidPath.Num() > 0)
                        {
                            FString StartCoords = FString::Printf(TEXT("(%d,%d)"), ActiveAI->CurrentTile->GridX, ActiveAI->CurrentTile->GridY);
                            FString EndCoords = FString::Printf(TEXT("(%d,%d)"), ValidPath.Last()->GridX, ValidPath.Last()->GridY);
                            UE_LOG(LogTemp, Warning, TEXT("AI: %s %s -> %s"), *ActiveAI->UnitID, *StartCoords, *EndCoords);

                            ActiveAI->MoveAlongPath(ValidPath);
                            bActionTaken = true;
                        }
                    }
                }
            }

            if (bActionTaken) break;
        }

        float Delay = bActionTaken ? 3.5f : 1.0f;
        DelaySwitchTurn(Delay);
        return;
    }

    // AI Deployment Phase
    if (bAIHasPlacedSecondUnit) return;

    TArray<AActor*> FoundTiles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGridTile::StaticClass(), FoundTiles);
    Algo::RandomShuffle(FoundTiles);

    for (AActor* ActorTile : FoundTiles)
    {
        if (AGridTile* Tile = Cast<AGridTile>(ActorTile))
        {
            if (Tile->GridY >= 22 && !Tile->bIsOccupied && Tile->bIsWalkable)
            {
                FVector SpawnLocation = Tile->GetActorLocation() + FVector(0.0f, 0.0f, 100.0f);
                FActorSpawnParameters SpawnParams;
                SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

                if (!bAIHasPlacedFirstUnit)
                {
                    if (AISniperClass)
                    {
                        AGameUnit* SpawnedSniper = GetWorld()->SpawnActor<AGameUnit>(AISniperClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
                        if (SpawnedSniper)
                        {
                            SpawnedSniper->CurrentTile = Tile;
                            SpawnedSniper->PlayerID = "AI";
                            SpawnedSniper->OwnerType = EUnitOwner::AI;
                        }
                        bAIHasPlacedFirstUnit = true;
                        Tile->bIsOccupied = true;
                        SwitchTurn();
                    }
                }
                else
                {
                    if (AIBrawlerClass)
                    {
                        AGameUnit* SpawnedBrawler = GetWorld()->SpawnActor<AGameUnit>(AIBrawlerClass, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
                        if (SpawnedBrawler)
                        {
                            SpawnedBrawler->CurrentTile = Tile;
                            SpawnedBrawler->PlayerID = "AI";
                            SpawnedBrawler->OwnerType = EUnitOwner::AI;
                        }
                        bAIHasPlacedSecondUnit = true;
                        Tile->bIsOccupied = true;
                        SwitchTurn();
                    }
                }
                break;
            }
        }
    }
}

void ATurnGameMode::DelaySwitchTurn(float DelayTime)
{
    bIsTurnTransitioning = true;
    GetWorldTimerManager().ClearTimer(TurnTransitionTimerHandle);
    GetWorldTimerManager().SetTimer(TurnTransitionTimerHandle, this, &ATurnGameMode::SwitchTurn, DelayTime, false);
}

void ATurnGameMode::RestartMatch()
{
    UE_LOG(LogTemp, Warning, TEXT("===================================="));
    UE_LOG(LogTemp, Warning, TEXT("INITIATING SYSTEM RESTART..."));
    UE_LOG(LogTemp, Warning, TEXT("===================================="));

    FName CurrentLevelName = FName(*UGameplayStatics::GetCurrentLevelName(GetWorld(), true));
    UGameplayStatics::OpenLevel(GetWorld(), CurrentLevelName);
}
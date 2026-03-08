#include "GameTower.h"
#include "GameUnit.h"
#include "GridTile.h"
#include "Components/StaticMeshComponent.h"
#include "Math/UnrealMathUtility.h"

AGameTower::AGameTower()
{
    PrimaryActorTick.bCanEverTick = false;

    TowerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TowerMesh"));
    RootComponent = TowerMesh;

    CurrentOwner = ETowerOwner::Neutral;
}

void AGameTower::EvaluateTowerState(const TArray<AGameUnit*>& AllUnits)
{
    if (!CurrentTile) return;

    bool bHumanInZone = false;
    bool bAIInZone = false;

    for (AGameUnit* Unit : AllUnits)
    {
        if (!Unit || !Unit->CurrentTile) continue;

        // Calculate Chebyshev distance to determine capture zone inclusion (Radius of 2)
        int32 DistX = FMath::Abs(Unit->CurrentTile->GridX - GridX);
        int32 DistY = FMath::Abs(Unit->CurrentTile->GridY - GridY);

        if (DistX <= 2 && DistY <= 2)
        {
            if (Unit->PlayerID == "HP") bHumanInZone = true;
            else if (Unit->PlayerID == "AI") bAIInZone = true;
        }
    }

    ETowerOwner OldOwner = CurrentOwner;

    if (bHumanInZone && bAIInZone)
    {
        CurrentOwner = ETowerOwner::Contested;
    }
    else if (bHumanInZone)
    {
        CurrentOwner = ETowerOwner::Human;
    }
    else if (bAIInZone)
    {
        CurrentOwner = ETowerOwner::AI;
    }
    else
    {
        CurrentOwner = ETowerOwner::Neutral;
    }

    if (OldOwner != CurrentOwner)
    {
        UpdateTowerVisuals();
    }
}

void AGameTower::UpdateTowerVisuals()
{
    FString StateString;
    UMaterialInterface* MaterialToApply = nullptr;

    switch (CurrentOwner)
    {
    case ETowerOwner::Neutral:
        StateString = TEXT("Neutral (Gray)");
        MaterialToApply = NeutralMaterial;
        break;
    case ETowerOwner::Human:
        StateString = TEXT("Human Controlled (Blue)");
        MaterialToApply = HumanMaterial;
        break;
    case ETowerOwner::AI:
        StateString = TEXT("AI Controlled (Red)");
        MaterialToApply = AIMaterial;
        break;
    case ETowerOwner::Contested:
        StateString = TEXT("Contested (Flashing)");
        MaterialToApply = ContestedMaterial;
        break;
    }

    UE_LOG(LogTemp, Warning, TEXT("Tower at (%d,%d) is now: %s"), GridX, GridY, *StateString);

    if (TowerMesh && MaterialToApply)
    {
        TowerMesh->SetMaterial(0, MaterialToApply);
    }
}
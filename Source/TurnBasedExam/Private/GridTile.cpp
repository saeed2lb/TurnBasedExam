#include "GridTile.h"
#include "Components/StaticMeshComponent.h"

AGridTile::AGridTile()
{
    PrimaryActorTick.bCanEverTick = false;

    TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
    RootComponent = TileMesh;

    bIsOccupied = false;
    ElevationLevel = 0;
    GridZ = 0;
}

void AGridTile::InitializeTile(int32 InX, int32 InY, int32 InElevation)
{
    GridX = InX;
    GridY = InY;
    ElevationLevel = InElevation;
    GridZ = InElevation;

    // Adjust vertical position based on terrain elevation scale
    FVector CurrentLocation = TileMesh->GetRelativeLocation();
    CurrentLocation.Z += (ElevationLevel * 50.0f);
    TileMesh->SetRelativeLocation(CurrentLocation);

    // Apply corresponding terrain material
    if (ElevationMaterials.IsValidIndex(ElevationLevel))
    {
        TileMesh->SetMaterial(0, ElevationMaterials[ElevationLevel]);
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Missing Material for Elevation Level %d!"), ElevationLevel);
    }
}
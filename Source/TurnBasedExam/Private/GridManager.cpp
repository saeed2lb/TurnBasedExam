#include "GridManager.h"
#include "GameTower.h"
#include "Engine/World.h"
#include "Algo/Reverse.h"
#include "Math/UnrealMathUtility.h"
#include "Kismet/GameplayStatics.h"

AGridManager::AGridManager()
{
    PrimaryActorTick.bCanEverTick = false;
    GridSize = 25;
    TileSize = 100.0f;
    NoiseSeed = FMath::RandRange(0.0f, 10000.0f);
}

void AGridManager::BeginPlay()
{
    Super::BeginPlay();
}

void AGridManager::GenerateGrid()
{
    if (!TileClassToSpawn) return;

    GridTiles.Empty();
    GridTiles.SetNumZeroed(GridSize * GridSize);

    FVector2D SeedOffset(NoiseSeed, NoiseSeed);

    for (int32 x = 0; x < GridSize; ++x)
    {
        for (int32 y = 0; y < GridSize; ++y)
        {
            FVector SpawnLocation(x * TileSize, y * TileSize, 0.0f);
            AGridTile* NewTile = GetWorld()->SpawnActor<AGridTile>(TileClassToSpawn, SpawnLocation, FRotator::ZeroRotator);

            if (NewTile)
            {
                float NoiseScale = 8.0f;
                FVector2D NoiseCoords = FVector2D(x / NoiseScale, y / NoiseScale) + SeedOffset;
                float RawNoise = FMath::PerlinNoise2D(NoiseCoords);
                float AmplifiedNoise = RawNoise * 1.7f;
                float NormalizedNoise = (AmplifiedNoise + 1.0f) / 2.0f;

                int32 Elevation = FMath::Clamp(FMath::RoundToInt(NormalizedNoise * 4.0f), 0, 4);

                NewTile->InitializeTile(x, y, Elevation);
                NewTile->bIsWalkable = (Elevation > 0);

                GridTiles[y * GridSize + x] = NewTile;
            }
        }
    }

    SpawnTowers();
    GeneratePathfindingGraph();
}

AGridTile* AGridManager::GetTileAt(int32 X, int32 Y)
{
    if (X < 0 || X >= GridSize || Y < 0 || Y >= GridSize) return nullptr;
    return GridTiles[Y * GridSize + X];
}

void AGridManager::SpawnTowers()
{
    if (!TowerClassToSpawn) return;

    TArray<FIntPoint> IdealCoordinates = {
        FIntPoint(5, 12),
        FIntPoint(12, 12),
        FIntPoint(19, 12)
    };

    for (FIntPoint IdealCoord : IdealCoordinates)
    {
        FIntPoint BestCoord = IdealCoord;
        AGridTile* TargetTile = GetTileAt(BestCoord.X, BestCoord.Y);

        // Adaptive placement: expand search if ideal tile is unwalkable/water
        int32 SearchRadius = 1;
        while (TargetTile == nullptr || TargetTile->ElevationLevel == 0)
        {
            bool bFound = false;
            for (int32 dx = -SearchRadius; dx <= SearchRadius && !bFound; ++dx)
            {
                for (int32 dy = -SearchRadius; dy <= SearchRadius && !bFound; ++dy)
                {
                    AGridTile* CheckTile = GetTileAt(IdealCoord.X + dx, IdealCoord.Y + dy);
                    if (CheckTile && CheckTile->ElevationLevel > 0)
                    {
                        TargetTile = CheckTile;
                        BestCoord = FIntPoint(IdealCoord.X + dx, IdealCoord.Y + dy);
                        bFound = true;
                    }
                }
            }
            SearchRadius++;
            if (SearchRadius > GridSize) break;
        }

        if (TargetTile)
        {
            float ZHeight = (TargetTile->ElevationLevel * 50.0f) + 50.0f;
            FVector SpawnLocation(BestCoord.X * TileSize, BestCoord.Y * TileSize, ZHeight);

            AGameTower* NewTower = GetWorld()->SpawnActor<AGameTower>(TowerClassToSpawn, SpawnLocation, FRotator::ZeroRotator);
            if (NewTower)
            {
                TargetTile->bIsWalkable = false;
                TargetTile->bIsOccupied = true;

                NewTower->CurrentTile = TargetTile;
                NewTower->GridX = BestCoord.X;
                NewTower->GridY = BestCoord.Y;
            }
        }
    }
}

AGridTile* AGridManager::GetTileAtPosition(int32 X, int32 Y)
{
    TArray<AActor*> FoundTiles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGridTile::StaticClass(), FoundTiles);

    for (AActor* ActorTile : FoundTiles)
    {
        AGridTile* Tile = Cast<AGridTile>(ActorTile);
        if (Tile && Tile->GridX == X && Tile->GridY == Y)
        {
            return Tile;
        }
    }
    return nullptr;
}

void AGridManager::GeneratePathfindingGraph()
{
    TArray<AActor*> FoundTiles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGridTile::StaticClass(), FoundTiles);

    for (AActor* ActorTile : FoundTiles)
    {
        AGridTile* CurrentTile = Cast<AGridTile>(ActorTile);

        if (CurrentTile && CurrentTile->bIsWalkable)
        {
            CurrentTile->Neighbors.Empty();

            AGridTile* NorthTile = GetTileAtPosition(CurrentTile->GridX, CurrentTile->GridY + 1);
            if (NorthTile && NorthTile->bIsWalkable) CurrentTile->Neighbors.Add(NorthTile);

            AGridTile* SouthTile = GetTileAtPosition(CurrentTile->GridX, CurrentTile->GridY - 1);
            if (SouthTile && SouthTile->bIsWalkable) CurrentTile->Neighbors.Add(SouthTile);

            AGridTile* EastTile = GetTileAtPosition(CurrentTile->GridX + 1, CurrentTile->GridY);
            if (EastTile && EastTile->bIsWalkable) CurrentTile->Neighbors.Add(EastTile);

            AGridTile* WestTile = GetTileAtPosition(CurrentTile->GridX - 1, CurrentTile->GridY);
            if (WestTile && WestTile->bIsWalkable) CurrentTile->Neighbors.Add(WestTile);
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("PATHFINDING GRAPH GENERATED: All tiles linked!"));
}

int32 AGridManager::GetManhattanDistance(AGridTile* TileA, AGridTile* TileB)
{
    int32 DistX = FMath::Abs(TileA->GridX - TileB->GridX);
    int32 DistY = FMath::Abs(TileA->GridY - TileB->GridY);
    return 10 * (DistX + DistY);
}

TArray<AGridTile*> AGridManager::FindPath(AGridTile* StartTile, AGridTile* TargetTile)
{
    TArray<AGridTile*> Path;

    if (!StartTile || !TargetTile || !TargetTile->bIsWalkable || TargetTile->bIsOccupied)
    {
        return Path;
    }

    // Reset previous pathfinding data
    TArray<AActor*> AllTiles;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGridTile::StaticClass(), AllTiles);
    for (AActor* ActorTile : AllTiles)
    {
        AGridTile* Tile = Cast<AGridTile>(ActorTile);
        if (Tile)
        {
            Tile->GCost = 0;
            Tile->HCost = 0;
            Tile->ParentTile = nullptr;
        }
    }

    TArray<AGridTile*> OpenSet;
    TArray<AGridTile*> ClosedSet;

    OpenSet.Add(StartTile);

    while (OpenSet.Num() > 0)
    {
        AGridTile* CurrentTile = OpenSet[0];
        for (int i = 1; i < OpenSet.Num(); i++)
        {
            if (OpenSet[i]->GetFCost() < CurrentTile->GetFCost() ||
                (OpenSet[i]->GetFCost() == CurrentTile->GetFCost() && OpenSet[i]->HCost < CurrentTile->HCost))
            {
                CurrentTile = OpenSet[i];
            }
        }

        OpenSet.Remove(CurrentTile);
        ClosedSet.Add(CurrentTile);

        // Target reached: retrace path
        if (CurrentTile == TargetTile)
        {
            AGridTile* RetraceTile = TargetTile;
            while (RetraceTile != StartTile)
            {
                Path.Add(RetraceTile);
                RetraceTile = RetraceTile->ParentTile;
            }

            Algo::Reverse(Path);
            return Path;
        }

        for (AGridTile* Neighbor : CurrentTile->Neighbors)
        {
            if (ClosedSet.Contains(Neighbor) || Neighbor->bIsOccupied)
            {
                continue;
            }

            int32 ElevationCost = (Neighbor->GridZ > CurrentTile->GridZ) ? 20 : 10;
            int32 NewMovementCostToNeighbor = CurrentTile->GCost + ElevationCost;

            if (NewMovementCostToNeighbor < Neighbor->GCost || !OpenSet.Contains(Neighbor))
            {
                Neighbor->GCost = NewMovementCostToNeighbor;
                Neighbor->HCost = GetManhattanDistance(Neighbor, TargetTile);
                Neighbor->ParentTile = CurrentTile;

                if (!OpenSet.Contains(Neighbor))
                {
                    OpenSet.Add(Neighbor);
                }
            }
        }
    }

    return Path;
}
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridTile.h" 
#include "GameTower.h" 
#include "GameUnit.h"
#include "GridManager.generated.h"

UCLASS()
class TURNBASEDEXAM_API AGridManager : public AActor
{
    GENERATED_BODY()

public:
    AGridManager();

protected:
    virtual void BeginPlay() override;

public:
    // --- Core Grid Settings ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    TSubclassOf<AGridTile> TileClassToSpawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    TSubclassOf<AGameTower> TowerClassToSpawn;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings", meta = (ClampMin = "1"))
    int32 GridSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings", meta = (ClampMin = "1.0"))
    float TileSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Settings")
    float NoiseSeed;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Data")
    TArray<AGridTile*> GridTiles;

    // --- Initialization & Spawning ---
    UFUNCTION(BlueprintCallable, Category = "Grid Functions")
    void GenerateGrid();

    UFUNCTION(BlueprintCallable, Category = "Grid Functions")
    void SpawnTowers();

    void GeneratePathfindingGraph();

    // --- Grid Utilities ---
    UFUNCTION(BlueprintCallable, Category = "Grid Functions")
    AGridTile* GetTileAt(int32 X, int32 Y);

    // --- Pathfinding ---
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<AGridTile*> FindPath(AGridTile* StartTile, AGridTile* TargetTile);

private:
    AGridTile* GetTileAtPosition(int32 X, int32 Y);
    int32 GetManhattanDistance(AGridTile* TileA, AGridTile* TileB);
};
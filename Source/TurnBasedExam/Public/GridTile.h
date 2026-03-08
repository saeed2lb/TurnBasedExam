#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridTile.generated.h"

UCLASS()
class TURNBASEDEXAM_API AGridTile : public AActor
{
    GENERATED_BODY()

public:
    AGridTile();

    void InitializeTile(int32 InX, int32 InY, int32 InElevation);

    // --- Core Components & Visuals ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* TileMesh;

    UPROPERTY(EditDefaultsOnly, Category = "Visuals")
    TArray<class UMaterialInterface*> ElevationMaterials;

    // --- Grid Coordinates & State ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Data")
    int32 GridX;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Data")
    int32 GridY;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Data")
    int32 GridZ;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid Data")
    int32 ElevationLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid State")
    bool bIsOccupied;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grid State")
    bool bIsWalkable;

    // --- A* Pathfinding Data ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
    TArray<AGridTile*> Neighbors;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
    AGridTile* ParentTile;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
    int32 GCost;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pathfinding")
    int32 HCost;

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    int32 GetFCost() const { return GCost + HCost; }
};
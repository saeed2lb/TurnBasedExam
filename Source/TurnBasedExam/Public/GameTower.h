#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameTower.generated.h"

class AGameUnit;
class AGridTile;

UENUM(BlueprintType)
enum class ETowerOwner : uint8
{
    Neutral     UMETA(DisplayName = "Neutral"),
    Human       UMETA(DisplayName = "Human Player"),
    AI          UMETA(DisplayName = "AI Player"),
    Contested   UMETA(DisplayName = "Contested")
};

UCLASS()
class TURNBASEDEXAM_API AGameTower : public AActor
{
    GENERATED_BODY()

public:
    AGameTower();

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* TowerMesh;

public:
    // --- Tower State & Data ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower Data")
    ETowerOwner CurrentOwner;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Data")
    AGridTile* CurrentTile;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Data")
    int32 GridX;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower Data")
    int32 GridY;

    // --- Visual Materials ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower Visuals")
    class UMaterialInterface* NeutralMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower Visuals")
    class UMaterialInterface* HumanMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower Visuals")
    class UMaterialInterface* AIMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tower Visuals")
    class UMaterialInterface* ContestedMaterial;

    // --- Core Logic ---
    UFUNCTION(BlueprintCallable, Category = "Tower Logic")
    void EvaluateTowerState(const TArray<AGameUnit*>& AllUnits);

    void UpdateTowerVisuals();
};
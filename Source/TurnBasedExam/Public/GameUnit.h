#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameUnit.generated.h"

class AGridTile;

UENUM(BlueprintType)
enum class EUnitOwner : uint8
{
    Human       UMETA(DisplayName = "Human Player"),
    AI          UMETA(DisplayName = "AI Player")
};

UENUM(BlueprintType)
enum class EAttackType : uint8
{
    Melee       UMETA(DisplayName = "Corto Raggio"),
    Ranged      UMETA(DisplayName = "Distanza")
};

UCLASS(Abstract)
class TURNBASEDEXAM_API AGameUnit : public AActor
{
    GENERATED_BODY()

public:
    AGameUnit();
    virtual void Tick(float DeltaTime) override;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Visuals")
    class UStaticMeshComponent* UnitMesh;

    bool bIsMoving;
    TArray<AGridTile*> CurrentPath;
    int32 CurrentPathIndex;

public:
    // --- Core Identifiers ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Data")
    EUnitOwner OwnerType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Data")
    FString PlayerID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Data")
    FString UnitID;

    // --- Grid Location ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Data")
    int32 GridX;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Grid Data")
    int32 GridY;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Grid Data")
    AGridTile* CurrentTile;

    // --- Combat & Movement Stats ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 MaxHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 CurrentHealth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 MovementRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    EAttackType AttackType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 AttackRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 MinDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    int32 MaxDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementSpeed = 400.0f;

    // --- Turn State Logic ---
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Turn Logic")
    bool bHasMovedThisTurn;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Turn Logic")
    bool bHasAttackedThisTurn;

    // --- Unit Actions ---
    UFUNCTION(BlueprintCallable, Category = "Turn Logic")
    void ResetActions();

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void MoveAlongPath(TArray<AGridTile*> PathToFollow);

    UFUNCTION(BlueprintCallable, Category = "Combat")
    virtual void PerformAttack(AGameUnit* Target);
};
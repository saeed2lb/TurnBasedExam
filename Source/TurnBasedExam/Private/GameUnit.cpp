#include "GameUnit.h"
#include "GridTile.h"

AGameUnit::AGameUnit()
{
    PrimaryActorTick.bCanEverTick = true;

    UnitMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UnitMesh"));
    RootComponent = UnitMesh;

    bIsMoving = false;
    CurrentPathIndex = 0;
    bHasMovedThisTurn = false;
    bHasAttackedThisTurn = false;
}

void AGameUnit::BeginPlay()
{
    Super::BeginPlay();
    CurrentHealth = MaxHealth;
}

void AGameUnit::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsMoving && CurrentPath.IsValidIndex(CurrentPathIndex))
    {
        AGridTile* TargetTile = CurrentPath[CurrentPathIndex];
        FVector TargetLocation = TargetTile->GetActorLocation() + FVector(0, 0, 100.0f);

        // Interpolate movement for smooth translation
        FVector NewLocation = FMath::VInterpConstantTo(GetActorLocation(), TargetLocation, DeltaTime, MovementSpeed);
        SetActorLocation(NewLocation);

        if (FVector::Dist(GetActorLocation(), TargetLocation) < 5.0f)
        {
            CurrentPathIndex++;

            if (CurrentPathIndex >= CurrentPath.Num())
            {
                bIsMoving = false;
                CurrentTile = TargetTile;
                CurrentTile->bIsOccupied = true;

                GridX = CurrentTile->GridX;
                GridY = CurrentTile->GridY;

                UE_LOG(LogTemp, Warning, TEXT("%s Movement Complete at (%d, %d)!"), *UnitID, GridX, GridY);
            }
        }
    }
}

void AGameUnit::MoveAlongPath(TArray<AGridTile*> PathToFollow)
{
    if (PathToFollow.Num() > 0)
    {
        CurrentPath = PathToFollow;
        CurrentPathIndex = 0;
        bIsMoving = true;
        bHasMovedThisTurn = true;

        if (CurrentTile)
        {
            CurrentTile->bIsOccupied = false;
        }
    }
}

void AGameUnit::PerformAttack(AGameUnit* Target)
{
    if (!Target || !CurrentTile || !Target->CurrentTile || bHasAttackedThisTurn) return;

    int32 DistX = FMath::Abs(CurrentTile->GridX - Target->CurrentTile->GridX);
    int32 DistY = FMath::Abs(CurrentTile->GridY - Target->CurrentTile->GridY);
    int32 Distance = DistX + DistY;

    if (Distance <= AttackRange)
    {
        // Elevation check: attackers cannot fire uphill
        if (Target->CurrentTile->GridZ <= CurrentTile->GridZ)
        {
            bHasAttackedThisTurn = true;

            int32 Damage = FMath::RandRange(MinDamage, MaxDamage);
            Target->CurrentHealth -= Damage;

            // Format Y-coordinate to character representation (e.g., A, B, C)
            char XLetter = 'A' + Target->CurrentTile->GridX;
            FString TargetCoords = FString::Printf(TEXT("%c%d"), XLetter, Target->CurrentTile->GridY);

            UE_LOG(LogTemp, Warning, TEXT("%s: %s %s %d"), *PlayerID, *UnitID, *TargetCoords, Damage);

            // Conditional counter-attack resolution
            if (UnitID == "S")
            {
                if (Target->UnitID == "S" || (Target->UnitID == "B" && Distance == 1))
                {
                    int32 CounterDamage = FMath::RandRange(1, 3);
                    CurrentHealth -= CounterDamage;
                    UE_LOG(LogTemp, Warning, TEXT("COUNTER-ATTACK! %s took %d damage!"), *UnitID, CounterDamage);
                }
            }

            if (Target->CurrentHealth <= 0)
            {
                Target->CurrentTile->bIsOccupied = false;
                Target->Destroy();
            }
        }
    }
}

void AGameUnit::ResetActions()
{
    bHasMovedThisTurn = false;
    bHasAttackedThisTurn = false;
}
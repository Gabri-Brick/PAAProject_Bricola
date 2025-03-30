#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"  
#include "Engine/CollisionProfile.h"         
#include "GridCell.generated.h"

UCLASS()
class STRATEGICO_API AGridCell : public AActor
{
    GENERATED_BODY()

public:
    AGridCell();


    void Init(int32 InX, int32 InY, bool bObstacle);

    /* Wrapper per impostare il materiale della cella */
    UFUNCTION(BlueprintCallable, Category = "GridCell")
    void SetCellMaterial(UMaterialInterface* NewMaterial);

    UFUNCTION()
    void OnCellClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);


    /** Coordinate della cella */
    UPROPERTY(BlueprintReadOnly, Category = "Grid")
    int32 GridX;

    UPROPERTY(BlueprintReadOnly, Category = "Grid")
    int32 GridY;

    /* Flag: true se la cella è un ostacolo, false altrimenti */
    UPROPERTY(BlueprintReadOnly, Category = "Grid")
    bool bIsObstacle;

    /* Componente Mesh per rappresentare la cella */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GridCell")
    UStaticMeshComponent* CellMesh;

    /* Materiale base per le celle libere */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridCell")
    UMaterialInterface* BaseMaterial;

    /* Materiale evidenziato (quando il mouse passa sopra) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridCell")
    UMaterialInterface* HighlightMaterial;

    /* Materiale per gli ostacoli */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GridCell")
    UMaterialInterface* ObstacleMaterial;

    /* Materiale originale assegnato alla cella libera (memorizzato al momento dell'inizializzazione) */
    UPROPERTY(BlueprintReadOnly, Category = "GridCell")
    UMaterialInterface* OriginalMaterial;

    // Variabili per utilità

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GridCell")
    bool bOccupied;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GridCell")
    bool bMovementHighlighted;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GridCell")
    bool bAttackHighlighted;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GridCell")
    class AUnit* OccupyingUnit;

    virtual void NotifyActorBeginCursorOver();
    virtual void NotifyActorEndCursorOver();
};

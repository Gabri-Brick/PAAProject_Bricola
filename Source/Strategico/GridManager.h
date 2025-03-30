#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GridCell.h"
#include "GridManager.generated.h"

// Forward declaration di AGridCell
class AGridCell;

UCLASS()
class AGridManager : public AActor
{
    GENERATED_BODY()

public:
    AGridManager();
    AGridCell* GetCellAt(int32 X, int32 Y) const;

protected:
    virtual void BeginPlay() override;

    /* Percentuale di celle che saranno ostacoli (0-100) */
    UPROPERTY(EditAnywhere, Category = "Grid Settings")
    int32 ObstaclePercentage;

    /* Materiale base per le celle libere (non ostacolo) */
    UPROPERTY(EditAnywhere, Category = "Grid Settings")
    UMaterialInterface* BaseMaterial;

    /* Lista di materiali disponibili per gli ostacoli (Scelti casualmente ma da immettere in unreal nella sezione dettagli) */
    UPROPERTY(EditAnywhere, Category = "Grid Settings")
    TArray<UMaterialInterface*> ObstacleMaterials;

    /* Materiale evidenziato per le celle libere (quando il mouse passa sopra) */
    UPROPERTY(EditAnywhere, Category = "Grid Settings")
    UMaterialInterface* HighlightMaterial;


    /* Dimensione (lato) della griglia in numero di celle (fisso a 25) */
    static const int32 GridSize = 25;

    /* Dimensione di ogni cella in unità Unreal */
    UPROPERTY(EditAnywhere, Category = "Grid Settings")
    float CellSize;

    /* Array delle celle generate */
    UPROPERTY(VisibleAnywhere, Category = "Grid Cells")
    TArray<AGridCell*> GridCells;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "GridCell")
    bool bOccupied;


};

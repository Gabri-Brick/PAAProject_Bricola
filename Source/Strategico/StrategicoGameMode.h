#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GridCell.h"
#include "Unit.h"
#include "HudStrategio.h"
#include "StrategicoGameMode.generated.h"

UENUM(BlueprintType)
enum class ETurnOwner : uint8
{
    Player  UMETA(DisplayName = "Giocatore"),
    AI      UMETA(DisplayName = "IA")
};

UENUM(BlueprintType)
enum class EGamePhase : uint8
{
    Placement UMETA(DisplayName = "Placement"),
    InGame    UMETA(DisplayName = "InGame"),
};


UCLASS()
class STRATEGICO_API AStrategicoGameMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AStrategicoGameMode();

    virtual void BeginPlay() override;

    /* Indica chi inizia il turno */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turni")
    ETurnOwner TurnOwner;

    /* Materiale per le unit� del giocatore (blu) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Materials")
    UMaterialInterface* PlayerUnitMaterial;

    /* Materiale per le unit� dell'IA (rosso) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Materials")
    UMaterialInterface* AIUnitMaterial;

    /* Numero di unit� posizionate dal giocatore nel turno corrente */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Placement")
    int32 PlayerUnitsPlaced;

    /* Numero di unit� posizionate dall'IA nel turno corrente */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Placement")
    int32 AIUnitsPlaced;

    //Propriet� per la GameFase corrente
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
    EGamePhase CurrentGamePhase = EGamePhase::Placement;

    // Materiali per le unit�
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Materials")
    UMaterialInterface* B_Sniper;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Materials")
    UMaterialInterface* B_Brawler;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Materials")
    UMaterialInterface* R_Sniper;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Materials")
    UMaterialInterface* R_Brawler;

    // Puntatore per gestire le eventuali unit� attive
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Turni")
    AUnit* ActiveUnit;

    /* Funzione per eseguire il lancio della moneta e determinare chi inizia */
    UFUNCTION(BlueprintCallable, Category = "Turni")
    void CoinToss();

    // Funzioni handler per gestire il click in base alla fase di gioco
    UFUNCTION(BlueprintCallable, Category = "Turni")
    void HandleCellClick(AGridCell* ClickedCell);

    UFUNCTION(BlueprintCallable, Category = "Turni")
    void HandleInGameCellClick(AGridCell* ClickedCell);

    /* Posiziona automaticamente le unit� dell'IA su celle libere */
    UFUNCTION(BlueprintCallable, Category = "Unit Placement")
    void AutoPlaceAIUnits();

    /* Posiziona una unit� del giocatore sulla cella target grazie al click precedentemente definito*/
    UFUNCTION(BlueprintCallable, Category = "Unit Placement")
    void PlacePlayerUnit(AGridCell* TargetCell);

    /* Passa al turno successivo */
    UFUNCTION(BlueprintCallable, Category = "Turni")
    void NextTurn();

    // Funzione per il calcolo del percorso per il movimento
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<AGridCell*> ComputePath(AGridCell* Start, AGridCell* Goal, AUnit* Unit);

    //Getter usato per passare le celle
    UFUNCTION(BlueprintCallable, Category = "Grid")
    AGridCell* GetGridCellAt(int32 X, int32 Y) const;

    //Funzione di utilit� per resettare le flag delle unit�
    UFUNCTION(BlueprintCallable, Category = "Turni")
    void ResetUnitsForNextTurn();

    //Funzione per determinare il comportamento dell'IA nel suo turno
    UFUNCTION(BlueprintCallable, Category = "Turni")
    void ProcessIATurn();

    // Restituisce tutte le unit� appartenenti al team IA
    UFUNCTION(BlueprintCallable, Category = "AI Utilities")
    TArray<class AUnit*> GetAllAIUnits() const;

    // Dato un'unit� IA, restituisce la unit� nemica pi� vicina (tra quelle del giocatore)
    UFUNCTION(BlueprintCallable, Category = "AI Utilities")
    AUnit* FindClosestEnemyUnit(AUnit* AIUnit) const;

    // Data un'unit� IA, restituisce true se esiste almeno un nemico in range d'attacco
    UFUNCTION(BlueprintCallable, Category = "AI Utilities")
    bool IsEnemyInAttackRange(AUnit* AIUnit) const;

    // Data un'unit� IA, restituisce una unit� nemica che � in range d'attacco (o nullptr se nessuno)
    UFUNCTION(BlueprintCallable, Category = "AI Utilities")
    AUnit* GetEnemyInAttackRange(AUnit* AIUnit) const;

    //Funzione helper per determinare se il movimento � stato completato
    UFUNCTION(BlueprintCallable, Category = "Turni")
    void AIMovementCompleted();

    //Funzione per la gestione del futuro movimento dell'IA
    UFUNCTION(BlueprintCallable, Category = "Turni")
    void ProcessNextAIMovement();

    //Gestione dell HUD
    UPROPERTY()
    UHudStrategio* HUD;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<UHudStrategio> HUDGame;

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void UpdateHUDHealthValues();

    //Funzione per determinare il GameOver
    UFUNCTION(BlueprintCallable, Category = "Game Over")
    void CheckForGameOver();

    //Getter per determinare l'identificativo della cella in coordinate AX
    UFUNCTION(BlueprintCallable, Category = "Grid")
    FString GetGridCellIdentifier(AGridCell* Cell) const;

    //Funzione per nascondere la griglia e le unit�
    UFUNCTION(BlueprintCallable, Category = "Grid")
    void HideGrid();

    //Gestione suoni
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Audio")
    USoundBase* BackgroundMusic;

    // Suono riprodotto in caso di vittoria
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
    USoundBase* VictorySound;

    // Suono riprodotto in caso di sconfitta
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
    USoundBase* LoseSound;

private:

    //Propriet� per gestire i movimenti dell'IA per determinare quante unit� si stanno muovendo
    int32 AIMovingUnitsCount;

    // Coda delle unit� IA in attesa di muoversi
    TArray<AUnit*> PendingAIMovementUnits;


};

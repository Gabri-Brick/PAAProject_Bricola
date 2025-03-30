#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Unit.generated.h"

// Enumerazione per il tipo di attacco
UENUM(BlueprintType)
enum class EAttackType : uint8
{
    Melee   UMETA(DisplayName = "Melee"),
    Ranged  UMETA(DisplayName = "Ranged")
};

// Enumerazione per il team (Player o IA)
UENUM(BlueprintType)
enum class EUnitTeam : uint8
{
    Player  UMETA(DisplayName = "Player"),
    AI      UMETA(DisplayName = "AI")
};

// Enumerazione per lo stato d'azione dell'unità
UENUM(BlueprintType)
enum class EUnitActionState : uint8
{
    Idle        UMETA(DisplayName = "Idle"),
    Movement    UMETA(DisplayName = "Movement"),
    Attack      UMETA(DisplayName = "Attack")
};

UCLASS()
class STRATEGICO_API AUnit : public APawn
{
    GENERATED_BODY()

public:
    AUnit();

    // Statistiche
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
    int32 MovimentoMax;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
    EAttackType AttackType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
    int32 RangeAttacco;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
    int32 DannoMin;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
    int32 DannoMax;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Stats")
    int32 VitaMax;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Stats")
    int32 Vita;

    UPROPERTY(BlueprintReadOnly, Category = "Unit")
    bool bHaMosso;

    UPROPERTY(BlueprintReadOnly, Category = "Unit")
    bool bHaAttaccato;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    EUnitTeam UnitTeam;

    // Stato d'azione attuale
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
    EUnitActionState CurrentActionState;

    // Mesh dell'unità
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
    UStaticMeshComponent* UnitMesh;

    //Materiale dell'unità
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
    UMaterialInterface* UnitMaterial;

    // Materiali da usare per evidenziare il range di movimento/attacco
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    UMaterialInterface* MovementHighlightMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    UMaterialInterface* AttackHighlightMaterial;

    // Array per salvare le celle evidenziate
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit")
    TArray<class AGridCell*> HighlightedCells;

    // Funzioni di base
    UFUNCTION(BlueprintCallable, Category = "Unit")
    virtual void SetUnitMaterial(UMaterialInterface* NewMaterial);

    //Funzione di attacco
    UFUNCTION(BlueprintCallable, Category = "Unit")
    virtual int32 Attack(class AUnit* TargetUnit);

    // Funzione per il toggle degli stati (macchina a stati)
    UFUNCTION(BlueprintCallable, Category = "Unit")
    void ToggleActionState();

    // Funzioni per evidenziare e nascondere gli highlight
    UFUNCTION(BlueprintCallable, Category = "Unit")
    void ShowMovementRange();

    UFUNCTION(BlueprintCallable, Category = "Unit")
    void ShowAttackRange();

    //Funzione di utilità per nascondere gli HighLight
    UFUNCTION(BlueprintCallable, Category = "Unit")
    void HideRanges();

    // Handler per il click sulla unità
    UFUNCTION()
    void OnUnitClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed);

    //Proprietà per determinare la cella corrente
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    class AGridCell* CurrentCell;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    bool bIsMoving;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    TArray<class AGridCell*> MovementPath;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
    int32 CurrentPathIndex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    float MovementSpeed;

    UFUNCTION(BlueprintCallable, Category = "Movement")
    void AnimateMovementAlongPath(const TArray<class AGridCell*>& Path);

    // Funzione chiamata al termine dell'animazione del movimento
    UFUNCTION(BlueprintCallable, Category = "Movement")
    void OnMovementFinished();

    // Override della funzione Tick per gestire l'animazione
    virtual void Tick(float DeltaTime) override;

    AGridCell* PendingTargetCell = nullptr;
    // Stato che verrà usato per alternare quando l'unità è in Idle e non ha ancora agito
    EUnitActionState LastDesiredActionState = EUnitActionState::Movement;

    // Nome da usare per la visualizzazione (ad esempio in HUD)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit")
    FString UnitDisplayName;

    //Suoni per gli attacchi
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    USoundBase* PunchSoundEffect;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
    USoundBase* SniperSoundEffect;

    // Suono riprodotto quando una unità viene eliminata
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sounds")
    USoundBase* KillConfirmedSound;


};

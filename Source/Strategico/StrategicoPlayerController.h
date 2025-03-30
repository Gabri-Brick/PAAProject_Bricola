#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "StrategicoPlayerController.generated.h"

/*
  PlayerController personalizzato per catturare l'input di tastiera,
  chiamare la funzione di cambio turno nel GameMode e gestire il movimento della camera.
 */
UCLASS()
class STRATEGICO_API AStrategicoPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    // Override del metodo SetupInputComponent per bindare le azioni e gli assi
    virtual void SetupInputComponent() override;

    // Funzione chiamata quando il giocatore preme il tasto per terminare il turno
    void OnEndTurnPressed();

    // Variabile per la velocità di movimento della camera
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Movement")
    float CameraMovementSpeed = 1000.0f;

    // Funzioni per muovere la camera (WASD)
    void MoveForward(float Value);
    void MoveRight(float Value);
};

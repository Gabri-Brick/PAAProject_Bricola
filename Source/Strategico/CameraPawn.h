#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CameraPawn.generated.h"

UCLASS()
class STRATEGICO_API ACameraPawn : public APawn
{
    GENERATED_BODY()

public:
    // Costruttore
    ACameraPawn();

    UFUNCTION()
    void ZoomCamera(float AxisValue);

protected:
    // Configura l'input (non serve per ora, la telecamera è fissa)
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
    /** Componente SpringArm (facoltativo, per controllare distanza e angolo della telecamera) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    class USpringArmComponent* SpringArm;

    /** Componente Camera */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
    class UCameraComponent* CameraComponent;
};

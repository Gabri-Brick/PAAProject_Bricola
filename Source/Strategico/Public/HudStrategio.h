#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "HudStrategio.generated.h"

UCLASS()
class STRATEGICO_API UHudStrategio : public UUserWidget
{
    GENERATED_BODY()

public:
    // Testo del turno 
    UPROPERTY(EditAnywhere, meta = (BindWidget))
    class UTextBlock* Turn_Text;

    // TextBlock per mostrare la salute e il nome delle unità:
    UPROPERTY(EditAnywhere, meta = (BindWidget))
    class UTextBlock* Txt_Health1;

    UPROPERTY(EditAnywhere, meta = (BindWidget))
    class UTextBlock* Txt_Health2;

    UPROPERTY(EditAnywhere, meta = (BindWidget))
    class UTextBlock* Txt_Health3;

    UPROPERTY(EditAnywhere, meta = (BindWidget))
    class UTextBlock* Txt_Health4;

    UPROPERTY(EditAnywhere, meta = (BindWidget))
    class UTextBlock* MoveHistoryText;

    UPROPERTY(EditAnywhere, meta = (BindWidget))
    class UScrollBox* MoveHistoryScrollBox;

    UPROPERTY(EditAnywhere, meta = (BindWidget))
    class UTextBlock* GameOverText;

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void ShowGameOverMessage(const FText& Message);


    UFUNCTION(BlueprintCallable, Category = "HUD")
    void AppendMoveHistory(const FString& NewEntry);

    // Funzione per settare il testo del turno
    void SetTurn(FText display);

    // Funzione per aggiornare i valori di salute (il testo che mostra "Nome: current/Max")
    void UpdateHealthTexts(const FString& Health1, const FString& Health2, const FString& Health3, const FString& Health4);

    UFUNCTION(BlueprintCallable, Category = "HUD")
    void HideHealthTexts();

};

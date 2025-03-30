#include "HudStrategio.h"
#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"

void UHudStrategio::SetTurn(FText display)
{
    if (Turn_Text)
    {
        Turn_Text->SetText(display);
    }
}

void UHudStrategio::UpdateHealthTexts(const FString& Health1, const FString& Health2, const FString& Health3, const FString& Health4)
{
    if (Txt_Health1)
        Txt_Health1->SetText(FText::FromString(Health1));
    if (Txt_Health2)
        Txt_Health2->SetText(FText::FromString(Health2));
    if (Txt_Health3)
        Txt_Health3->SetText(FText::FromString(Health3));
    if (Txt_Health4)
        Txt_Health4->SetText(FText::FromString(Health4));
}

void UHudStrategio::AppendMoveHistory(const FString& NewEntry)
{
    if (!MoveHistoryText) return;

    // 1. Recupera il testo corrente
    FString CurrentText = MoveHistoryText->GetText().ToString();

    // 2. Aggiungi una nuova riga
    CurrentText.Append("\n");
    CurrentText.Append(NewEntry);

    // 3. Imposta il testo aggiornato
    MoveHistoryText->SetText(FText::FromString(CurrentText));

    // 4. Fai l’auto-scroll verso la fine, se lo ScrollBox esiste
    if (MoveHistoryScrollBox)
    {
        MoveHistoryScrollBox->ScrollToEnd();
    }
}

void UHudStrategio::ShowGameOverMessage(const FText& Message)
{
    // Assicurati che il TextBlock sia valido
    if (GameOverText)  // GameOverText è il nome del TextBlock nel blueprint
    {
        GameOverText->SetText(Message);
        // Cambia la visibilità in Visible
        GameOverText->SetVisibility(ESlateVisibility::Visible);
    }
}

void UHudStrategio::HideHealthTexts()
{
    if (Txt_Health1)
    {
        Txt_Health1->SetVisibility(ESlateVisibility::Hidden);
    }
    if (Txt_Health2)
    {
        Txt_Health2->SetVisibility(ESlateVisibility::Hidden);
    }
    if (Txt_Health3)
    {
        Txt_Health3->SetVisibility(ESlateVisibility::Hidden);
    }
    if (Txt_Health4)
    {
        Txt_Health4->SetVisibility(ESlateVisibility::Hidden);
    }
}

# TurnBasedExam
# Progetto Strategico a Turni 3D - Esame PAA 2025/2026
**Sviluppato in:** Unreal Engine 5.6 (C++)
**Studente:** [IL TUO NOME E COGNOME]

## Requisiti Implementati (10/10)

1. **Generazione Mappa (Perlin Noise):** OK. Griglia 25x25 generata proceduralmente con seed configurabile all'avvio. 5 livelli di elevazione con colori corrispondenti (Blu, Verde, Giallo, Arancio, Rosso).
2. **Widget di Configurazione (UMG):** OK. All'avvio è presente un widget per l'inserimento del seed di generazione.
3. **Posizionamento Torri:** OK. 3 Torri posizionate simmetricamente con algoritmo adattivo per evitare zone d'acqua.
4. **Fase di Schieramento (Iniziativa):** OK. Lancio della moneta 50/50 per determinare chi inizia. Schieramento vincolato alle prime due righe per lato.
5. **A* Pathfinding e Stamina:** OK. Implementato algoritmo A* in C++. Costo base 10, salita 20. Controllo stamina (MovementRange) implementato.
6. **Combattimento ed Elevazione:** OK. Rispetto del raggio d'azione (Melee/Ranged). Gli attacchi verso l'alto sono bloccati come da specifica.
7. **Danno da Contrattacco:** OK. Sniper subisce danni se attacca un altro Sniper o un Brawler a distanza 1.
8. **Cattura Torri (Area 2x2):** OK. Area di cattura basata su distanza di Chebyshev (<=2). Stato "Contested" implementato se presenti entrambe le fazioni.
9. **Vittoria per Dominio:** OK. Vittoria assegnata dopo il controllo di 2 torri per 2 turni consecutivi.
10. **Log di Gioco (Naming Specifica):** OK. Log dettagliati nel formato richiesto (es. "HP: S B4 -> D6") per ogni movimento e attacco.

## Istruzioni
- All'avvio inserire un seed numerico nel widget per generare la mappa.
- Posizionare lo Sniper e il Brawler cliccando sulla griglia (prime due righe).
- Al termine del gioco, il match si riavvia automaticamente dopo 5 secondi.
#include "Application.h"
#include "imgui/imgui.h"
#include "classes/Chess.h"

namespace ClassGame {
        //
        // our global variables
        //
        Chess *game = nullptr;
        bool gameOver = false;
        int gameWinner = -1;

        //
        // game starting point
        // this is called by the main render loop in main.cpp
        //
        void GameStartUp() 
        {
            game = new Chess();
            game->setUpBoard();
            game->setAIPlayer(1);
            game->updateAI();
        }

        //
        // game render loop
        // this is called by the main render loop in main.cpp
        //
        void RenderGame() 
        {
                ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());

                ImGui::ShowDemoWindow();

                ImGui::Begin("Settings");
                ImGui::Text("Current Player Number: %d", game->getCurrentPlayer()->playerNumber());
                ImGui::Text("Current Board State: %s", game->stateString().c_str());

                if (gameOver) {
                    ImGui::Text("Game Over!");
                    if (gameWinner == -1)
                        ImGui::Text("Draw.");
                    else if (gameWinner)
                        ImGui::Text("Black wins.");
                    else
                        ImGui::Text("White wins.");
                    
                    if (ImGui::Button("Reset Game")) {
                        game->stopGame();
                        game->setUpBoard();
                        gameOver = false;
                        gameWinner = -1;
                    }
                }
                ImGui::End();

                ImGui::Begin("GameWindow");
                game->drawFrame();
                ImGui::End();
        }

        //
        // end turn is called by the game code at the end of each turn
        // this is where we check for a winner
        //
        void EndOfTurn() 
        {
            // dont remove it pisses off the cpp gods
            Player*winner = game->checkForWinner();
            if (winner){
                gameOver = true;
                gameWinner = winner->playerNumber();
            }else if (game->checkForDraw()) {
                gameOver = true;
                gameWinner = -1;
            }else
                game->updateAI();
        }
}

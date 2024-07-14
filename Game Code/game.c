#include <stdio.h>
#include <stdlib.h>
#include "game.h"
#include "enemy.h"

void initGame(Game *game)
{
    initShip(&game->ship);
    game->bullets = NULL;
    game->bullet_count = 0;

    game->total_active_enemy_ships = 0;

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        initEnemyBullet(&game->enemy_bullets[i]);
    }

    game->game_over = 0; // Inicializa el juego como no terminado
}

void checkCollisions(Game *game)
{
    // Verificar colisiones entre balas del jugador y nave enemiga
    for (int i = 0; i < game->bullet_count; i++)
    {
        Bullet *bullet = &game->bullets[i];
        if (bullet->active)
        {
            for (int j = 0; j < game->total_active_enemy_ships; j++)
            {
                if (game->enemies[j].active && bullet->x == game->enemies[j].x && bullet->y == game->enemies[j].y)
                {
                    bullet->active = 0;
                    game->enemies[j].active = 0;
                }
            }
        }
    }

    // Verificar colisiones entre balas enemigas y la nave del jugador
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        EnemyBullet *bullet = &game->enemy_bullets[i];
        if (bullet->active && bullet->x == game->ship.x && bullet->y == game->ship.y)
        {
            bullet->active = 0;
            game->ship.lives--;
            if (game->ship.lives <= 0)
            {
                game->game_over = 1; // Termina el juego si las vidas llegan a cero
            }
        }
    }

    // Verificar colisiones entre balas
    for (int i = 0; i < game->bullet_count; i++)
    {
        if (game->bullets[i].active)
        {
            for (int j = 0; j < MAX_ENEMY_BULLETS; j++)
            {
                if (game->enemy_bullets[j].active)
                {
                    // Verificar colisión en la misma posición
                    if (game->bullets[i].x == game->enemy_bullets[j].x &&
                        game->bullets[i].y == game->enemy_bullets[j].y)
                    {
                        game->bullets[i].active = 0;
                        game->enemy_bullets[j].active = 0;
                        printf("\033[%d;%dH*", game->bullets[i].y + 1, game->bullets[i].x + 1); // Mostrar colisión
                    }
                    // Verificar colisión cruzada inminente
                    else if (game->bullets[i].x == game->enemy_bullets[j].x &&
                             game->bullets[i].y == game->enemy_bullets[j].y + 1)
                    {
                        game->bullets[i].active = 0;
                        game->enemy_bullets[j].active = 0;
                        printf("\033[%d;%dH*", game->bullets[i].y + 1, game->bullets[i].x + 1);             // Mostrar colisión
                        printf("\033[%d;%dH*", game->enemy_bullets[j].y + 2, game->enemy_bullets[j].x + 1); // Mostrar colisión
                    }
                }
            }
        }
    }
}

void updateGame(Game *game, char input)
{
    if (game->game_over)
    {
        return; // No actualizar si el juego ha terminado
    }

    // Actualizar la nave del jugador
    updateShip(&game->ship, input);

    if (input == 'k')
    {
        addBullet(game, game->ship.x, game->ship.y - 1);
    }

    // Actualizar balas del jugador
    for (int i = 0; i < game->bullet_count; i++)
    {
        updateBullet(&game->bullets[i]);
    }

    // Actualizar enemigos
    for (int i = 0; i < game->total_active_enemy_ships; i++)
    {
        NaveEnemiga naveEnemiga = game->enemies[i];
        if (naveEnemiga.active == 1)
        {
            naveEnemiga.update(&naveEnemiga);   // Actualiza el enemigo
            naveEnemiga.moveDown(&naveEnemiga); // Mover enemigos hacia abajo
            if (naveEnemiga.moveSide)
            {
                naveEnemiga.moveSide(&naveEnemiga); // Mover lateralmente si está habilitado
            }

            // Verificar si el enemigo ha llegado a la tierra
            if (naveEnemiga.y >= FIELD_HEIGHT - 1)
            {
                naveEnemiga.active = 0;
                game->ship.lives--;
                if (game->ship.lives <= 0)
                {
                    game->game_over = 1; // Termina el juego si las vidas llegan a cero
                }
            }
        }

        game->enemies[i] = naveEnemiga;
    }

    // Lógica para disparar la nave enemiga (por ejemplo, cada cierto tiempo)
    static int fire_counter = 0;
    if (fire_counter++ > 8)
    {
        for (int i = 0; i < game->total_active_enemy_ships; i++)
        {
            NaveEnemiga naveEnemiga = game->enemies[i];
            if (naveEnemiga.active)
            {
                naveEnemiga.fire(&naveEnemiga, game->enemy_bullets);
            }
        }
        fire_counter = 0;
    }

    // Actualizar balas enemigas
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        updateEnemyBullet(&game->enemy_bullets[i]);
    }

    // Generar enemigos random
    if (rand() % 100 <= 7)
    {
        if (game->total_active_enemy_ships < ENEMY_TOTAL_AMOUNT)
        {
            static NaveEnemiga naveEnemiga;
            initGeneralEnemy(&naveEnemiga);

            game->enemies[game->total_active_enemy_ships] = naveEnemiga;
            game->total_active_enemy_ships++;
        }
    }

    // Verificar colisiones
    checkCollisions(game);
}

void renderGame(Game *game)
{
    system("cls");

    // Dibuja los bordes del campo con letras del alfabeto
    for (int i = 0; i <= FIELD_WIDTH + 1; i++)
    {
        printf("%c", ALPHABET_UPPER[i % 26]);
    }
    printf("\n");
    for (int i = 0; i < FIELD_HEIGHT; i++)
    {
        printf("%c", ALPHABET_UPPER[i % 26]);
        for (int j = 0; j < FIELD_WIDTH; j++)
        {
            printf(" ");
        }
        printf("%c\n", ALPHABET_UPPER[i % 26]);
    }
    for (int i = 0; i <= FIELD_WIDTH + 1; i++)
    {
        printf("%c", ALPHABET_UPPER[i % 26]);
    }
    printf("\n");

    // Dibuja la nave y los disparos del jugador
    renderShip(&game->ship);
    for (int i = 0; i < game->bullet_count; i++)
    {
        renderBullet(&game->bullets[i]);
    }

    // Dibuja los enemigos
    for (int i = 0; i < game->total_active_enemy_ships; i++)
    {
        NaveEnemiga naveEnemiga = game->enemies[i];
        naveEnemiga.render(&naveEnemiga);
    }
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++)
    {
        renderEnemyBullet(&game->enemy_bullets[i]);
    }

    // Mostrar vidas del jugador
    printf("\033[%d;%dHLIVES: %d\n", FIELD_HEIGHT + 3, 0, game->ship.lives);

    // Mostrar mensaje de fin de juego si es necesario
    if (game->game_over)
    {
        printf("\033[%d;%dHGame Over\n", FIELD_HEIGHT / 2, FIELD_WIDTH / 2 - 4);
    }
}

void addBullet(Game *game, int x, int y)
{
    game->bullets = realloc(game->bullets, (game->bullet_count + 1) * sizeof(Bullet));
    initBullet(&game->bullets[game->bullet_count]);
    game->bullets[game->bullet_count].x = x;
    game->bullets[game->bullet_count].y = y;
    game->bullets[game->bullet_count].active = 1;
    game->bullet_count++;
}

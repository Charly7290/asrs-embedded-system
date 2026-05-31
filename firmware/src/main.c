#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app/logger.h"
#include "app/system.h"
 
void app_main(void)
{
    // Logger primero — antes de cualquier otra cosa
    logger_init();
 
    // Pequeña pausa para que el logger arranque su tarea
    vTaskDelay(pdMS_TO_TICKS(100));
 
    LOG_I("BOOT", ERR_OK, "app_main iniciando");
 
    // Lanzar la maquina de estados
    system_init();
 
    // app_main NO debe retornar — si retorna, FreeRTOS mata las tareas
    // Mantenerlo vivo con un loop
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}
# ASRS Embedded System

Prototipo de sistema embebido para un **Sistema Automatizado de Almacenamiento y Recuperación (AS/RS – Automated Storage and Retrieval System)** desarrollado como proyecto del curso de Sistemas Embebidos en la Universidad EIA.

---

# Introducción

Los sistemas automatizados de almacenamiento y recuperación (AS/RS) son ampliamente utilizados en entornos industriales y logísticos para mejorar la eficiencia en la gestión de inventarios, reducir errores humanos y optimizar el uso del espacio de almacenamiento. Estos sistemas permiten almacenar, clasificar y recuperar productos mediante mecanismos automatizados controlados por sistemas electrónicos y de software.

En este proyecto se propone el diseño e implementación de un sistema embebido capaz de controlar un prototipo compacto de AS/RS, implementando sensado, actuación, protocolos de comunicación y una interfaz de usuario para gestionar las operaciones del sistema.

El sistema permitirá a un operario almacenar, recuperar e intercambiar productos dentro de diferentes compartimentos, manteniendo registro del estado del sistema y validando las operaciones mediante sensores y mediciones físicas.

El desarrollo del sistema seguirá buenas prácticas de ingeniería en hardware y firmware, incluyendo diseño arquitectónico, manejo de errores, trazabilidad de requisitos, validación mediante pruebas y documentación técnica formal.

---

# Descripción del Problema

En muchos entornos de almacenamiento de pequeña y mediana escala, como laboratorios, bodegas técnicas o entornos educativos, la gestión de inventarios se realiza de forma manual.

Este proceso presenta diversos problemas:

* Pérdida de tiempo en la búsqueda de productos.
* Errores humanos al ubicar o retirar elementos.
* Falta de control sobre la presencia o ausencia de productos en los compartimentos.
* Dificultad para mantener registros confiables del estado del sistema.

Los sistemas comerciales de AS/RS solucionan estos problemas, pero suelen tener alto costo y complejidad, lo que limita su implementación en entornos educativos o prototipos experimentales.

Por esta razón, este proyecto propone el desarrollo de un prototipo de sistema automatizado de almacenamiento y recuperación controlado por un sistema embebido, capaz de gestionar de forma automática el almacenamiento y recuperación de productos dentro de una estructura de compartimentos.

---

# Alcance del Proyecto

El proyecto contempla el desarrollo de un prototipo funcional de un sistema AS/RS compacto controlado por un sistema embebido, capaz de gestionar el almacenamiento y recuperación de productos.

El sistema incluirá los siguientes componentes:

* Un **mecanismo de posicionamiento** capaz de acceder a diferentes compartimentos de almacenamiento.
* **Sensores de presencia** para determinar si un compartimento contiene un producto.
* **Celdas de carga** para medir el peso del producto almacenado.
* **Actuadores controlados electrónicamente** para el movimiento del sistema.
* **Protocolos de comunicación** entre los módulos electrónicos (UART, SPI o I2C).
* Un **sistema de logging** que registre eventos, estados y errores del sistema.
* Una **interfaz gráfica de usuario (GUI)** que permita:

  * Configurar parámetros del sistema.
  * Visualizar el estado del sistema.
  * Consultar errores o eventos relevantes.

La implementación física del sistema incluirá:

* Tarjeta electrónica ensamblada.
* Fuente de alimentación externa.
* Estructura mecánica funcional que represente el sistema de almacenamiento.
* Carcasa o estructura de protección.

El proyecto se limitará al desarrollo de un prototipo demostrativo a escala, orientado a validar la arquitectura y funcionamiento del sistema embebido.

---

# Objetivos

## Objetivo General

Diseñar e implementar un sistema embebido para el control de un sistema automatizado de almacenamiento y recuperación (AS/RS) que permita gestionar operaciones de almacenamiento, recuperación e intercambio de productos mediante sensores, actuadores y una interfaz de usuario.

---

## Objetivos Específicos

1. Diseñar la arquitectura de hardware y firmware del sistema embebido.
2. Implementar mecanismos de sensado de presencia y medición de peso en los compartimentos.
3. Desarrollar el control de actuadores para el posicionamiento del sistema.
4. Implementar un sistema de logging estructurado que registre eventos, estados y errores.
5. Integrar protocolos de comunicación entre módulos electrónicos (UART, SPI o I2C).
6. Diseñar e implementar una interfaz gráfica de usuario (GUI) para interacción con el sistema.
7. Implementar mecanismos robustos de manejo de errores y validación de sensores.
8. Realizar pruebas funcionales del sistema, asegurando la trazabilidad entre requisitos y pruebas.

---

# Asignación de Roles

El equipo de desarrollo adoptará una estructura de roles similar a la utilizada en entornos profesionales de ingeniería.

## Technical Lead

Responsabilidades:

* Definir la arquitectura general del sistema.
* Aprobar decisiones técnicas del proyecto.
* Garantizar coherencia entre requisitos, diseño e implementación.
* Supervisar la integración del sistema.
* Administrar el repositorio del proyecto.

Integrante:
Carlos Daniel Murillo Mena

---

## Firmware Engineer

Responsabilidades:

* Desarrollo de los módulos de firmware.
* Implementación del control de sensores y actuadores.
* Desarrollo del sistema de logging.
* Implementación de los protocolos de comunicación.
* Documentación del código fuente.

Integrante:
Owen Montiel Cardeñas

---

## Hardware Integration Engineer

Responsabilidades:

* Diseño del esquemático electrónico.
* Integración de sensores, actuadores y microcontrolador.
* Diseño de PCB o tarjeta universal soldada.
* Validación eléctrica del sistema.
* Ensamblaje físico y diseño de la carcasa.

Integrante:
Mateo Aldana Escobar

---

## Verification & Testing Engineer

Responsabilidades:

* Definir el plan de pruebas del sistema.
* Construir y mantener la SRTM (Software Requirements Traceability Matrix).
* Validar los requisitos mediante casos de prueba.
* Recolectar evidencia del funcionamiento del sistema.

Integrante:
Michael Cuello Perez

---

# Información del Proyecto

Proyecto de Sistemas Embebidos  
Universidad EIA  
Periodo académico **2026-1**

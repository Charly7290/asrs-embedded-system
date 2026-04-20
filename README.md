# AS/RS Embedded System

Prototipo de sistema embebido para un **Sistema Automatizado de Almacenamiento y Recuperación (AS/RS – Automated Storage and Retrieval System)** desarrollado como proyecto del curso de Sistemas Embebidos en la Universidad EIA.

> **Estado del proyecto:** Entrega 3/4 — Diagrama de bloques, esquemático y BOM.
> Entrega final: semana del 18–22 de mayo de 2026.

---

## Estructura del repositorio

```
asrs-embedded/
├── docs/                    # Documentación del proyecto
│   ├── firmware-design/     # Embedded Firmware Design Document
│   ├── test-cases/evidence/ # Evidencia de pruebas ejecutadas
│   └── images/              # Diagramas, fotos, capturas
├── firmware/                # Proyecto PlatformIO
│   ├── src/                 # Código fuente
│   │   ├── hal/             # Abstracción de hardware
│   │   ├── drivers/         # Drivers (stepper, hx711, mcp23017)
│   │   ├── services/        # Lógica de negocio (motion, inventory, logger)
│   │   └── app/             # Máquina de estados principal
│   ├── include/             # Headers globales (config, error codes)
│   ├── lib/                 # Librerías externas
│   └── test/                # Unit tests
├── hardware/                # Diseño electrónico
│   ├── schematic/           # Proyecto KiCad
│   ├── pcb/                 # Diseño de PCB y Gerbers
│   ├── enclosure/           # Diseño CAD de la carcasa
│   └── photos/              # Fotos del ensamblaje
└── gui/                     # Interfaz gráfica de usuario
```

---

## Cómo compilar

### Requisitos

- [VS Code](https://code.visualstudio.com/) con la extensión [PlatformIO](https://platformio.org/)
- Cable USB para ESP32

### Pasos

```bash
git clone https://github.com/<usuario>/asrs-embedded.git
cd asrs-embedded
# Abrir con VS Code — PlatformIO detecta el proyecto automáticamente
# Compilar: Ctrl+Alt+B o botón ✓ en la barra de PlatformIO
# Subir al ESP32: Ctrl+Alt+U o botón →
# Monitor serial: Ctrl+Alt+S o botón 🔌
```

---

## Introducción

Los sistemas automatizados de almacenamiento y recuperación (AS/RS) son ampliamente utilizados en entornos industriales y logísticos para mejorar la eficiencia en la gestión de inventarios, reducir errores humanos y optimizar el uso del espacio de almacenamiento. Estos sistemas permiten almacenar, clasificar y recuperar productos mediante mecanismos automatizados controlados por sistemas electrónicos y de software.

En este proyecto se propone el diseño e implementación de un sistema embebido capaz de controlar un prototipo compacto de AS/RS, implementando sensado, actuación, protocolos de comunicación y una interfaz de usuario para gestionar las operaciones del sistema.

El sistema permitirá a un operario almacenar, recuperar e intercambiar productos dentro de diferentes compartimentos, manteniendo registro del estado del sistema y validando las operaciones mediante sensores y mediciones físicas.

El desarrollo del sistema seguirá buenas prácticas de ingeniería en hardware y firmware, incluyendo diseño arquitectónico, manejo de errores, trazabilidad de requisitos, validación mediante pruebas y documentación técnica formal.

---

## Descripción del Problema

En muchos entornos de almacenamiento de pequeña y mediana escala, como laboratorios, bodegas técnicas o entornos educativos, la gestión de inventarios se realiza de forma manual.

Este proceso presenta diversos problemas:

- Pérdida de tiempo en la búsqueda de productos.
- Errores humanos al ubicar o retirar elementos.
- Falta de control sobre la presencia o ausencia de productos en los compartimentos.
- Dificultad para mantener registros confiables del estado del sistema.

Los sistemas comerciales de AS/RS solucionan estos problemas, pero suelen tener alto costo y complejidad, lo que limita su implementación en entornos educativos o prototipos experimentales.

Por esta razón, este proyecto propone el desarrollo de un prototipo de sistema automatizado de almacenamiento y recuperación controlado por un sistema embebido, capaz de gestionar de forma automática el almacenamiento y recuperación de productos dentro de una estructura de compartimentos.

---

## Alcance del Proyecto

El proyecto contempla el desarrollo de un prototipo funcional de un sistema AS/RS compacto controlado por un sistema embebido, capaz de gestionar el almacenamiento y recuperación de productos.

El sistema incluirá los siguientes componentes:

- Un **mecanismo de posicionamiento** capaz de acceder a diferentes compartimentos de almacenamiento.
- **Sensores de presencia** para determinar si un compartimento contiene un producto.
- **Celdas de carga** para medir el peso del producto almacenado.
- **Actuadores controlados electrónicamente** para el movimiento del sistema.
- **Protocolos de comunicación** entre los módulos electrónicos (UART e I2C).
- Un **sistema de logging** que registre eventos, estados y errores del sistema.
- Una **interfaz gráfica de usuario (GUI)** que permita configurar parámetros, visualizar el estado del sistema y consultar errores o eventos relevantes.

La implementación física incluirá tarjeta electrónica ensamblada, fuente de alimentación externa, estructura mecánica funcional y carcasa de protección.

El proyecto se limita al desarrollo de un prototipo demostrativo a escala, orientado a validar la arquitectura y funcionamiento del sistema embebido.

---

## Objetivos

### Objetivo General

Diseñar e implementar un sistema embebido para el control de un sistema automatizado de almacenamiento y recuperación (AS/RS) que permita gestionar operaciones de almacenamiento, recuperación e intercambio de productos mediante sensores, actuadores y una interfaz de usuario.

### Objetivos Específicos

1. Diseñar la arquitectura de hardware y firmware del sistema embebido.
2. Implementar mecanismos de sensado de presencia y medición de peso en los compartimentos.
3. Desarrollar el control de actuadores para el posicionamiento del sistema.
4. Implementar un sistema de logging estructurado que registre eventos, estados y errores.
5. Integrar protocolos de comunicación entre módulos electrónicos (UART e I2C).
6. Diseñar e implementar una interfaz gráfica de usuario (GUI) para interacción con el sistema.
7. Implementar mecanismos robustos de manejo de errores y validación de sensores.
8. Realizar pruebas funcionales del sistema, asegurando la trazabilidad entre requisitos y pruebas.

---

## Equipo de Desarrollo

### Technical Lead — Carlos Daniel Murillo Mena
- Definir la arquitectura general del sistema.
- Aprobar decisiones técnicas del proyecto.
- Garantizar coherencia entre requisitos, diseño e implementación.
- Supervisar la integración y administrar el repositorio.

### Firmware Engineer — Owen Montiel Cardeñas
- Desarrollo de los módulos de firmware.
- Implementación del control de sensores y actuadores.
- Desarrollo del sistema de logging y protocolos de comunicación.
- Documentación del código fuente.

### Hardware Integration Engineer — Mateo Aldana Escobar
- Diseño del esquemático electrónico y PCB.
- Integración de sensores, actuadores y microcontrolador.
- Validación eléctrica del sistema.
- Ensamblaje físico y diseño de la carcasa.

### Verification & Testing Engineer — Michael Cuello Perez
- Definir el plan de pruebas del sistema.
- Construir y mantener la SRTM.
- Validar los requisitos mediante casos de prueba.
- Recolectar evidencia del funcionamiento del sistema.

---

## Entregas

| # | Entregable | Fecha | Estado |
|---|---|---|---|
| 1 | README + roles + problema | 11/03/2026 | ✅ |
| 2 | SRS + plan de testing + SRTM | 27/03/2026 | ✅ |
| 3 | Diagrama de bloques + esquemático + BOM | 17/04/2026 | ✅ |
| 4 | Entrega final + demo | 18–22/05/2026 | ⬜ |

---

*Proyecto de Sistemas Embebidos — Universidad EIA — Periodo académico 2026-1*

#!/bin/bash
# ============================================================
# AS/RS Embedded System — Repo Structure Setup
# Universidad EIA — Sistemas Embebidos 2026-1
# Ejecutar desde la RAÍZ del repositorio git
# Usage: bash setup_repo.sh
# ============================================================

set -e

if [ ! -d ".git" ]; then
    echo "ERROR: No se encontró directorio .git."
    echo "Ejecuta este script desde la raíz de tu repositorio."
    exit 1
fi

echo "Creando estructura de carpetas..."

# docs/
mkdir -p docs/firmware-design
mkdir -p docs/test-cases/evidence
mkdir -p docs/images

# firmware/ (estructura PlatformIO)
mkdir -p firmware/src/hal
mkdir -p firmware/src/drivers
mkdir -p firmware/src/services
mkdir -p firmware/src/app
mkdir -p firmware/include
mkdir -p firmware/lib
mkdir -p firmware/test

# hardware/
mkdir -p hardware/schematic
mkdir -p hardware/pcb
mkdir -p hardware/enclosure
mkdir -p hardware/photos

# gui/
mkdir -p gui

# .gitkeep en carpetas vacías
find . -type d -empty -not -path './.git/*' -exec touch {}/.gitkeep \;

# --- .gitignore ---
cat > .gitignore << 'EOF'
# PlatformIO
.pio/
.pioenvs/
.piolibdeps/
.vscode/.browse.c_cpp.db*
.vscode/c_cpp_properties.json
.vscode/launch.json
.vscode/ipch

# KiCad
*.kicad_sch-bak
*.kicad_pcb-bak
*-backups/
fp-info-cache
*_autosave-*

# Proteus / Multisim
*.DBK
*.LBK
*.ms14.bak

# Build
build/
*.o
*.elf
*.bin
*.hex
*.map

# OS
.DS_Store
Thumbs.db
desktop.ini
*.swp
*~

# IDE
.idea/
*.code-workspace

# Python (GUI)
__pycache__/
*.pyc
venv/
.env
EOF

# --- CONTRIBUTING.md ---
cat > CONTRIBUTING.md << 'EOF'
# Guía de contribución — AS/RS Embedded System

## Branching model

- **`main`** — Siempre compilable. Solo se actualiza mediante PR aprobado.
- **`develop`** — Rama de integración.
- **`feat/RF-XX-descripcion`** — Features vinculados a un requisito.
- **`fix/descripcion`** — Correcciones de bugs.
- **`docs/descripcion`** — Cambios de documentación.

### Flujo

1. Crear rama desde `develop`:
   ```
   git checkout develop
   git pull origin develop
   git checkout -b feat/RF-03-control-stepper
   ```
2. Commits atómicos con convención.
3. Pull Request hacia `develop` con al menos un revisor.

## Convención de commits

```
<tipo>(<alcance>): <descripción corta>
```

| Tipo       | Uso                                    |
|------------|----------------------------------------|
| `feat`     | Nueva funcionalidad o módulo           |
| `fix`      | Corrección de bug                      |
| `docs`     | Solo documentación                     |
| `test`     | Agregar o modificar tests              |
| `refactor` | Cambio sin cambiar comportamiento      |
| `hw`       | Cambios en esquemático, PCB o BOM      |
| `chore`    | Mantenimiento del repo                 |

### Ejemplos

```
feat(drivers): implementar driver HX711 con lectura a 10 SPS
fix(motion): corregir cálculo de pasos en eje Y
docs(srs): agregar criterio de aceptación a RF-07
test(TC-003): agregar evidencia de peso con pesa patrón 100g
hw(schematic): agregar pull-ups I2C en bus del MCP23017
```

## Evidencia de test cases

Guardar en `docs/test-cases/evidence/` con la convención:

```
TC-XXX_YYYYMMDD_tipo.ext
```

Ejemplo: `TC-003_20260510_serial-capture.png`
EOF

# --- CHANGELOG.md ---
cat > CHANGELOG.md << 'EOF'
# Changelog — AS/RS Embedded System

## [Entrega 1] - 2026-03-11

### Agregado
- README.md con definición del problema, roles, alcance y objetivos.

## [Entrega 2] - 2026-03-27

### Agregado
- SRS con 13 requisitos funcionales y 6 no funcionales.
- Plan de testing por 5 niveles.
- SRTM inicial con 19 requisitos mapeados.

## [Entrega 3] - 2026-04-17

### Agregado
- (Actualizar con lo entregado)
EOF

echo ""
echo "Listo. Estructura creada."
echo ""
echo "Pasos siguientes:"
echo "  1. git add -A"
echo "  2. git commit -m 'chore: setup project structure'"
echo "  3. git push origin main"
echo "  4. git checkout -b develop && git push -u origin develop"

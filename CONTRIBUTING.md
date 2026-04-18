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

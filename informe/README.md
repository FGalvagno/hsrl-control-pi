# Informe Final - Proyecto Integrador

Sistema Automatizado de Control de Longitud de Onda para LiDAR de Alta
Resolución Espectral (HSRL).

## Estructura

```
informe/
├── main.tex                  Documento maestro
├── portada.tex
├── preliminares/              Aprobación, aval, dedicatoria, agradecimientos,
│                               resumen, abstract, resumo, lista de símbolos
├── capitulos/
│   ├── cap01_introduccion.tex
│   ├── cap02_lidar_hsrl.tex            Parte I - Marco Teórico
│   ├── cap03_fotodeteccion.tex
│   ├── cap04_sistemas_embebidos.tex
│   ├── cap05_control_hmi.tex
│   ├── cap06_deteccion_picos.tex       Parte II - Marco Metodológico
│   ├── cap07_placa_control.tex
│   ├── cap08_firmware.tex
│   ├── cap09_software_hmi.tex
│   └── cap10_modelo_experimental.tex
├── resultados.tex            Parte III
├── conclusiones.tex          Parte III
├── bibliografia.bib
├── anexos/
│   ├── anexoI_hojas_datos.tex
│   ├── anexoII_manual_seeder.tex
│   ├── anexoIII_codigo.tex
│   ├── sat.tex
│   └── informes_mensuales.tex
└── figuras/
```

## Compilación

Requiere una distribución TeX con `biber` (TeX Live o MiKTeX).

```
pdflatex main.tex
biber main
pdflatex main.tex
pdflatex main.tex
```

## Formato

Basado en el Manual de Estilo para el Informe Final de la Cátedra Proyecto
Integrador (FCEFyN - UNC): A4, Times New Roman 11, interlineado 1,5,
márgenes 3 cm (izquierdo) / 2,5 cm (resto), numeración romana en
preliminares y arábiga desde el Capítulo 1.

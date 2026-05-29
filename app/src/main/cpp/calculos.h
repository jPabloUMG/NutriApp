#ifndef NUTRIAPP_CALCULOS_H
#define NUTRIAPP_CALCULOS_H

#include "database.h"

// Resultados del análisis nutricional
struct ResultadoNutricional {
    float imc;
    std::string categoriaIMC;
    float tmb;
    float caloriasRecomendadas;
    float caloriasConsumidas;
    float diferenciaCalorias;
    float carbsConsumidos;
    float proteinasConsumidas;
    float grasasConsumidas;
    bool excesoCalorias;
    bool deficitCalorias;
    bool excesoGrasas;
    bool excesoAzucares;
};

// Cálculo de IMC
float calcularIMC(float peso, float altura);
std::string categoriaIMC(float imc);

// Cálculo de TMB (Tasa Metabólica Basal)
float calcularTMB(float peso, float altura, int edad, std::string genero);

// Cálculo de calorías recomendadas según actividad
float calcularCaloriasRecomendadas(float tmb, std::string actividad);

// Análisis nutricional completo
ResultadoNutricional analizarNutricion(
        const Usuario& usuario,
        float caloriasConsumidas,
        float carbsConsumidos,
        float proteinasConsumidas,
        float grasasConsumidas,
        float azucaresConsumidos
);

// Generar texto de recomendaciones
std::string generarRecomendaciones(const ResultadoNutricional& resultado);

#endif //NUTRIAPP_CALCULOS_H

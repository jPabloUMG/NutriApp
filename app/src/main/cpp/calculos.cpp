#include "calculos.h"
#include <cmath>
#include <sstream>

float calcularIMC(float peso, float altura) {
    if (altura <= 0) return 0;
    float alturaMetros = altura / 100.0f;
    return peso / (alturaMetros * alturaMetros);
}

std::string categoriaIMC(float imc) {
    if (imc < 18.5f) return "Bajo peso";
    if (imc < 25.0f) return "Normal";
    if (imc < 30.0f) return "Sobrepeso";
    return "Obesidad";
}

float calcularTMB(float peso, float altura, int edad, std::string genero) {
    // Fórmula de Harris-Benedict
    if (genero == "masculino" || genero == "hombre") {
        return 88.362f + (13.397f * peso) + (4.799f * altura) - (5.677f * edad);
    } else {
        return 447.593f + (9.247f * peso) + (3.098f * altura) - (4.330f * edad);
    }
}

float calcularCaloriasRecomendadas(float tmb, std::string actividad) {
    float factor = 1.2f; // sedentario por defecto
    if (actividad == "ligero")    factor = 1.375f;
    if (actividad == "moderado")  factor = 1.55f;
    if (actividad == "activo")    factor = 1.725f;
    if (actividad == "muy activo") factor = 1.9f;
    return tmb * factor;
}

ResultadoNutricional analizarNutricion(
        const Usuario& usuario,
        float caloriasConsumidas,
        float carbsConsumidos,
        float proteinasConsumidas,
        float grasasConsumidas,
        float azucaresConsumidos
) {
    ResultadoNutricional resultado;

    // Cálculos principales
    resultado.imc = calcularIMC(usuario.peso, usuario.altura);
    resultado.categoriaIMC = categoriaIMC(resultado.imc);
    resultado.tmb = calcularTMB(usuario.peso, usuario.altura,
                                usuario.edad, usuario.genero);
    resultado.caloriasRecomendadas = calcularCaloriasRecomendadas(
            resultado.tmb, usuario.actividad);

    // Balance calórico
    resultado.caloriasConsumidas = caloriasConsumidas;
    resultado.diferenciaCalorias = caloriasConsumidas - resultado.caloriasRecomendadas;
    resultado.carbsConsumidos = carbsConsumidos;
    resultado.proteinasConsumidas = proteinasConsumidas;
    resultado.grasasConsumidas = grasasConsumidas;

    // Alertas hola
    resultado.excesoCalorias  = resultado.diferenciaCalorias > 200;
    resultado.deficitCalorias = resultado.diferenciaCalorias < -200;
    resultado.excesoGrasas    = grasasConsumidas > 70;
    resultado.excesoAzucares  = azucaresConsumidos > 50;

    return resultado;
}

std::string generarRecomendaciones(const ResultadoNutricional& resultado) {
    std::stringstream rec;

    rec << "=== RECOMENDACIONES NUTRICIONALES ===\n\n";
    rec << "IMC: " << resultado.imc << " (" << resultado.categoriaIMC << ")\n";
    rec << "Calorias recomendadas: " << resultado.caloriasRecomendadas << " kcal\n";
    rec << "Calorias consumidas: " << resultado.caloriasConsumidas << " kcal\n\n";

    if (resultado.excesoCalorias)
        rec << "⚠ Estas consumiendo mas calorias de las recomendadas.\n";
    if (resultado.deficitCalorias)
        rec << "⚠ Estas consumiendo menos calorias de las recomendadas.\n";
    if (resultado.excesoGrasas)
        rec << "⚠ Alto consumo de grasas detectado.\n";
    if (resultado.excesoAzucares)
        rec << "⚠ Alto consumo de azucares detectado.\n";

    if (!resultado.excesoCalorias && !resultado.deficitCalorias
        && !resultado.excesoGrasas && !resultado.excesoAzucares)
        rec << "✓ Tu dieta esta dentro de los parametros saludables.\n";

    return rec.str();
}
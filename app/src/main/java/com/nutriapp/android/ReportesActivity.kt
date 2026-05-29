package com.nutriapp.android

import android.content.Intent
import android.os.Bundle
import android.util.Log
import android.widget.Button
import androidx.appcompat.app.AppCompatActivity
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import java.text.ParseException
import java.text.SimpleDateFormat
import java.util.*

class ReportesActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_reportes)

        val recyclerComidas = findViewById<RecyclerView>(R.id.recyclerComidas)
        recyclerComidas.layoutManager = LinearLayoutManager(this)

        val btnVolver = findViewById<Button>(R.id.btnVolver)
        btnVolver.setOnClickListener {
            startActivity(Intent(this, MainActivity::class.java))
            finish()
        }

        val btnRecomendaciones = findViewById<Button>(R.id.btnRecomendaciones)
        btnRecomendaciones.setOnClickListener {
            val prefs = getSharedPreferences("NutriApp", MODE_PRIVATE)
            val usuarioId = prefs.getInt("usuario_id", -1)

            // ✅ Ajuste: usar dd/MM/yyyy porque así viene de la BD
            val fechaHoy = SimpleDateFormat("dd/MM/yyyy", Locale.getDefault()).format(Date())
            val comidas = obtenerComidasDesdeNativo(usuarioId)

            // Depuración opcional
            comidas.forEach {
                Log.d("NutriApp", "DEBUG -> Nombre=${it.nombre}, Fecha=${it.fecha}, Tiempo=${it.tiempoComida}, Calorias=${it.calorias}")
            }

            var totalCalorias = 0f
            var totalProteinas = 0f
            var totalCarbos = 0f
            var totalGrasas = 0f

            val comidasHoy = comidas.filter { it.fecha == fechaHoy }

            comidasHoy.forEach {
                val kcal = if (it.calorias > 0) it.calorias else (it.proteinas * 4 + it.carbohidratos * 4 + it.grasas * 9)
                totalCalorias += kcal
                totalProteinas += it.proteinas
                totalCarbos += it.carbohidratos
                totalGrasas += it.grasas
            }

            if (totalCalorias <= 0f) {
                android.app.AlertDialog.Builder(this)
                    .setTitle("Recomendaciones Nutricionales")
                    .setMessage("⚠ No hay datos de consumo registrados para hoy.")
                    .setPositiveButton("OK", null)
                    .show()
                return@setOnClickListener
            }

            val kcalProteinas = totalProteinas * 4
            val kcalCarbos    = totalCarbos * 4
            val kcalGrasas    = totalGrasas * 9

            val porcentajeProteinas = (kcalProteinas / totalCalorias) * 100
            val porcentajeCarbos    = (kcalCarbos / totalCalorias) * 100
            val porcentajeGrasas    = (kcalGrasas / totalCalorias) * 100

            val recomendaciones = mutableListOf<String>()
            if (porcentajeProteinas < 15f) recomendaciones.add("⚠ Tus proteínas son bajas (${porcentajeProteinas.toInt()}%). Incluye más carnes magras, huevos o legumbres.")
            if (porcentajeCarbos > 60f) recomendaciones.add("⚠ Exceso de carbohidratos (${porcentajeCarbos.toInt()}%). Controla pan, pasta y arroz.")
            if (porcentajeGrasas > 35f) recomendaciones.add("⚠ Alto consumo de grasas (${porcentajeGrasas.toInt()}%). Reduce frituras y embutidos.")
            if (totalCalorias > 3500f) recomendaciones.add("⚠ Consumo calórico muy alto (${totalCalorias.toInt()} kcal). Procura balancear con actividad física.")

            if (recomendaciones.isEmpty()) recomendaciones.add("✓ Tu consumo está dentro de parámetros saludables.")

            android.app.AlertDialog.Builder(this)
                .setTitle("Recomendaciones Nutricionales")
                .setMessage(recomendaciones.joinToString("\n"))
                .setPositiveButton("OK", null)
                .show()
        }








        val prefs = getSharedPreferences("NutriApp", MODE_PRIVATE)
        val usuarioId = prefs.getInt("usuario_id", -1)

        val comidas = obtenerComidasDesdeNativo(usuarioId)
        Log.d("NutriApp", "Comidas parseadas: ${comidas.size}")
        recyclerComidas.adapter = ComidaAdapter(comidas)
    }

    private fun obtenerComidasDesdeNativo(usuarioId: Int): List<Comida> {
        val resumen = obtenerResumenReportes(usuarioId)
        Log.d("NutriApp", "Resumen raw:\n$resumen")

        val lineas = resumen.split("\n").map { it.trim() }.filter { it.isNotEmpty() }

        return lineas.mapNotNull { linea ->
            try {
                val fuenteRegex = Regex("""\[(.*?)\]\s*\z""")
                val fuenteMatch = fuenteRegex.find(linea)
                var fuente = "manual"
                var lineaSinFuente = linea
                if (fuenteMatch != null) {
                    fuente = fuenteMatch.groupValues[1].trim()
                    lineaSinFuente = linea.substring(0, fuenteMatch.range.first).trim()
                }

                val caloriasRegex = Regex("""\((\d+(?:\.\d+)?)\s*kcal\)""", RegexOption.IGNORE_CASE)
                val caloriasMatch = caloriasRegex.find(lineaSinFuente)
                val calorias = caloriasMatch?.groupValues?.get(1)?.toFloatOrNull() ?: 0f
                var lineaLimpia = lineaSinFuente.replace(caloriasRegex, "").trim()

                var proteinas = 0f
                var carbohidratos = 0f
                var grasas = 0f
                val proteinasRegex = Regex("""Prote[ií]nas?\s*[:\-]?\s*(\d+(?:\.\d+)?)\s*g""", RegexOption.IGNORE_CASE)
                val carbsRegex = Regex("""Carbohidratos?\s*[:\-]?\s*(\d+(?:\.\d+)?)\s*g""", RegexOption.IGNORE_CASE)
                val grasasRegex = Regex("""Grasas?\s*[:\-]?\s*(\d+(?:\.\d+)?)\s*g""", RegexOption.IGNORE_CASE)

                proteinasRegex.find(lineaLimpia)?.let { proteinas = it.groupValues[1].toFloatOrNull() ?: 0f }
                carbsRegex.find(lineaLimpia)?.let { carbohidratos = it.groupValues[1].toFloatOrNull() ?: 0f }
                grasasRegex.find(lineaLimpia)?.let { grasas = it.groupValues[1].toFloatOrNull() ?: 0f }

                lineaLimpia = lineaLimpia
                    .replace(proteinasRegex, "")
                    .replace(carbsRegex, "")
                    .replace(grasasRegex, "")
                    .trim()

                var cantidad = "1 porción"
                val cantidadRegex = Regex("""\b(\d+\s*(?:porciones?|porción))\b""", RegexOption.IGNORE_CASE)
                val cantidadMatch = cantidadRegex.find(lineaLimpia)
                if (cantidadMatch != null) {
                    cantidad = cantidadMatch.groupValues[1].trim()
                    lineaLimpia = lineaLimpia.replace(cantidadMatch.value, "").trim().trim(',', '-').trim()
                }

                var fechaRaw = ""
                var tiempo = ""

                val fechaCandidate = Regex("""\d{1,2}[\/\-]\d{1,2}[\/\-]\d{2,4}""").find(lineaLimpia)
                if (fechaCandidate != null) {
                    fechaRaw = fechaCandidate.value
                    lineaLimpia = lineaLimpia.replace(fechaCandidate.value, "").trim()
                }

                val tipoRegex = Regex("""(?i)\b(desayuno|almuerzo|cena|merienda)\b""")
                val tipoMatch = tipoRegex.find(lineaLimpia)
                if (tipoMatch != null) {
                    tiempo = tipoMatch.groupValues[1].replaceFirstChar { it.uppercase() }
                    lineaLimpia = lineaLimpia.replace(tipoMatch.value, "").trim()
                }

                // NUEVA LIMPIEZA ABSOLUTA: Borra cualquier residuo de números sueltos, horas, dos puntos y guiones al inicio del texto
                lineaLimpia = lineaLimpia.replace(Regex("""^\s*\d+[\s:]*\d*[\s:\-\–\—,]*"""), "").trim()
                lineaLimpia = lineaLimpia.replace(Regex("""^[:\s\-\–\—,]+"""), "").trim()

                val nombre = lineaLimpia.ifEmpty { "Sin nombre" }
                val fechaFinal = if (fechaRaw.isNotEmpty()) formatearFecha(fechaRaw) else ""

                return@mapNotNull Comida(
                    fechaFinal,
                    tiempo,
                    nombre,
                    cantidad,
                    calorias,
                    proteinas,
                    carbohidratos,
                    grasas,
                    fuente
                )
            } catch (e: Exception) {
                Log.e("NutriApp", "Error parseando línea: $linea -> ${e.message}")
                return@mapNotNull null
            }
        }
    }

    private fun formatearFecha(raw: String): String {
        val trimmed = raw.trim()
        if (trimmed.isEmpty()) return ""

        // SOLUCIÓN DEFINITIVA: Desarmamos el formato invertido de C++ (Año/Mes/Día)
        // Ejemplo: "26/05/0027" -> Grupo 1: "26" (Año), Grupo 2: "05" (Mes), Grupo 3: "27" (Día)
        val regexNativo = Regex("""^(\d{2})[/\-](\d{2})[/\-](?:00)?(\d{2})$""")
        val match = regexNativo.find(trimmed)

        if (match != null) {
            val ano = match.groupValues[1]  // "26"
            val mes = match.groupValues[2]  // "05"
            val dia = match.groupValues[3]  // "27"

            // Lo rearmamos correctamente como Día/Mes/Año completo
            return "$dia/$mes/20$ano" // Resultado: "27/05/2026"
        }

        // Sistema de respaldo por si acaso entra una fecha con formato tradicional
        val formatosEntrada = listOf("dd/MM/yyyy", "yyyy-MM-dd", "dd-MM-yyyy")
        for (fmt in formatosEntrada) {
            try {
                val sdfIn = SimpleDateFormat(fmt, Locale.getDefault())
                sdfIn.isLenient = false
                val date = sdfIn.parse(trimmed)
                if (date != null) {
                    val sdfOut = SimpleDateFormat("dd/MM/yyyy", Locale.getDefault())
                    return sdfOut.format(date)
                }
            } catch (ignored: Exception) { }
        }

        return trimmed
    }

    external fun obtenerResumenReportes(usuarioId: Int): String

    companion object {
        init { System.loadLibrary("nutriapp") }
    }
}
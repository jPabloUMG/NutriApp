package com.nutriapp.android

import android.content.Intent
import android.graphics.Color
import android.os.Bundle
import android.widget.Button
import android.widget.ProgressBar
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        val prefs     = getSharedPreferences("NutriApp", MODE_PRIVATE)
        val usuarioId = prefs.getInt("usuario_id", -1)

        if (usuarioId == -1) {
            startActivity(Intent(this, LoginActivity::class.java))
            finish()
            return
        }

        val dbPath = filesDir.absolutePath + "/nutriapp.db"
        inicializarDB(dbPath)

        // Referencias vistas
        val tvSaludo              = findViewById<TextView>(R.id.tvSaludo)
        val tvFecha               = findViewById<TextView>(R.id.tvFecha)
        val tvCalorias            = findViewById<TextView>(R.id.tvCalorias)
        val tvCaloriasObjetivo    = findViewById<TextView>(R.id.tvCaloriasObjetivo)
        val tvCaloriasRestantes   = findViewById<TextView>(R.id.tvCaloriasRestantes)
        val progressCalorias      = findViewById<ProgressBar>(R.id.progressCalorias)
        val tvIMCValor            = findViewById<TextView>(R.id.tvIMCValor)
        val tvIMCCategoria        = findViewById<TextView>(R.id.tvIMCCategoria)
        val tvGrasaValor          = findViewById<TextView>(R.id.tvGrasaValor)
        val tvGrasaCategoria      = findViewById<TextView>(R.id.tvGrasaCategoria)
        val tvGrasaTituloTabla    = findViewById<TextView>(R.id.tvGrasaTituloTabla)
        val tvGrasaRango1         = findViewById<TextView>(R.id.tvGrasaRango1)
        val tvGrasaRango2         = findViewById<TextView>(R.id.tvGrasaRango2)
        val tvGrasaRango3         = findViewById<TextView>(R.id.tvGrasaRango3)
        val tvGrasaRango4         = findViewById<TextView>(R.id.tvGrasaRango4)
        val tvGrasaRango5         = findViewById<TextView>(R.id.tvGrasaRango5)
        val tvTMB                 = findViewById<TextView>(R.id.tvTMB)
        val tvCaloriasObjetivoCard= findViewById<TextView>(R.id.tvCaloriasObjetivoCard)
        val tvCaloriasConsumidas  = findViewById<TextView>(R.id.tvCaloriasConsumidas)
        val tvCaloriasEstado      = findViewById<TextView>(R.id.tvCaloriasEstado)
        val tvPesoTotal           = findViewById<TextView>(R.id.tvPesoTotal)
        val tvMasaMagra           = findViewById<TextView>(R.id.tvMasaMagra)
        val tvMasaMuscular        = findViewById<TextView>(R.id.tvMasaMuscular)
        val tvMasaGrasa           = findViewById<TextView>(R.id.tvMasaGrasa)
        val tvAgua                = findViewById<TextView>(R.id.tvAgua)
        val tvMasaOsea            = findViewById<TextView>(R.id.tvMasaOsea)
        val progressMasaMagra     = findViewById<ProgressBar>(R.id.progressMasaMagra)
        val progressMasaMuscular  = findViewById<ProgressBar>(R.id.progressMasaMuscular)
        val progressMasaGrasa     = findViewById<ProgressBar>(R.id.progressMasaGrasa)
        val tvEdadMetabolica      = findViewById<TextView>(R.id.tvEdadMetabolica)
        val tvGrasaVisceral       = findViewById<TextView>(R.id.tvGrasaVisceral)
        val tvPuntuacion          = findViewById<TextView>(R.id.tvPuntuacion)
        val tvTipoCuerpo          = findViewById<TextView>(R.id.tvTipoCuerpo)
        val btnRegistrarComida    = findViewById<Button>(R.id.btnRegistrarComida)
        val btnReportes           = findViewById<Button>(R.id.btnReportes)

        // Fecha
        val hoy = SimpleDateFormat("EEEE, dd 'de' MMMM", Locale("es")).format(Date())
        tvFecha.text = hoy.replaceFirstChar { it.uppercase() }

        // Perfil
        val perfil = obtenerPerfilUsuario(usuarioId)
        val genero = perfil[0]  // "Masculino" o "Femenino"
        val nombre = perfil[1]
        tvSaludo.text = "¡Hola, ${nombre.split(" ")[0]}!"

        // Última medición
        val medicion = obtenerUltimaMedicion(usuarioId)
        val tieneMedicion = medicion[0] != "-1"

        if (tieneMedicion) {
            val imc          = medicion[0].toFloatOrNull() ?: 0f
            val peso         = medicion[1].toFloatOrNull() ?: 0f
            val grasa        = medicion[2].toFloatOrNull() ?: 0f
            val calObjetivo  = medicion[3].toFloatOrNull() ?: 0f
            val masaMagra    = medicion[4].toFloatOrNull() ?: 0f
            val masaMuscular = medicion[5].toFloatOrNull() ?: 0f
            val masaGrasa    = medicion[6].toFloatOrNull() ?: 0f
            val masaOsea     = medicion[7].toFloatOrNull() ?: 0f
            val agua         = medicion[8].toFloatOrNull() ?: 0f
            val grasaVis     = medicion[9].toFloatOrNull() ?: 0f
            val edadMet      = medicion[10].toIntOrNull() ?: 0
            val puntuacion   = medicion[11].toFloatOrNull() ?: 0f
            val tipoCuerpo   = medicion[12]
            val tmb          = medicion[13].toFloatOrNull() ?: 0f

            // ── IMC ──────────────────────────────────────────────
            tvIMCValor.text = "%.1f".format(imc)
            val (imcLabel, imcColor) = when {
                imc < 18.5f -> Pair("⚠ Bajo peso", "#2196F3")
                imc < 25.0f -> Pair("✓ Peso normal", "#4CAF50")
                imc < 30.0f -> Pair("⚠ Sobrepeso", "#FF9800")
                else        -> Pair("✗ Obesidad", "#F44336")
            }
            tvIMCCategoria.text = imcLabel
            tvIMCCategoria.setBackgroundColor(Color.parseColor(imcColor))
            tvIMCValor.setTextColor(Color.parseColor(imcColor))

            // ── % GRASA ───────────────────────────────────────────
            tvGrasaValor.text = "%.1f %%".format(grasa)

            val esHombre = genero == "Masculino"
            if (esHombre) {
                tvGrasaTituloTabla.text = "Rangos ACE/OMS (Hombre)"
                tvGrasaRango1.text = "2–5%"
                tvGrasaRango2.text = "6–13%"
                tvGrasaRango3.text = "14–17%"
                tvGrasaRango4.text = "18–24%"
                tvGrasaRango5.text = "≥25%"
                val (grasaLabel, grasaColor) = when {
                    grasa < 6f  -> Pair("Esencial", "#9C27B0")
                    grasa < 14f -> Pair("✓ Atlético", "#2196F3")
                    grasa < 18f -> Pair("✓ Fitness", "#4CAF50")
                    grasa < 25f -> Pair("⚠ Promedio", "#FF9800")
                    else        -> Pair("✗ Obeso", "#F44336")
                }
                tvGrasaCategoria.text = grasaLabel
                tvGrasaCategoria.setBackgroundColor(Color.parseColor(grasaColor))
                tvGrasaValor.setTextColor(Color.parseColor(grasaColor))
            } else {
                tvGrasaTituloTabla.text = "Rangos ACE/OMS (Mujer)"
                tvGrasaRango1.text = "10–13%"
                tvGrasaRango2.text = "14–20%"
                tvGrasaRango3.text = "21–24%"
                tvGrasaRango4.text = "25–31%"
                tvGrasaRango5.text = "≥32%"
                val (grasaLabel, grasaColor) = when {
                    grasa < 14f -> Pair("Esencial", "#9C27B0")
                    grasa < 21f -> Pair("✓ Atlético", "#2196F3")
                    grasa < 25f -> Pair("✓ Fitness", "#4CAF50")
                    grasa < 32f -> Pair("⚠ Promedio", "#FF9800")
                    else        -> Pair("✗ Obeso", "#F44336")
                }
                tvGrasaCategoria.text = grasaLabel
                tvGrasaCategoria.setBackgroundColor(Color.parseColor(grasaColor))
                tvGrasaValor.setTextColor(Color.parseColor(grasaColor))
            }

            // ── TMB Y CALORÍAS ────────────────────────────────────
            tvTMB.text = "%.0f".format(tmb)
            tvCaloriasObjetivoCard.text = "%.0f".format(calObjetivo)
            tvCaloriasObjetivo.text = "Objetivo: %.0f kcal".format(calObjetivo)

            // ── COMPOSICIÓN CORPORAL ──────────────────────────────
            tvPesoTotal.text    = "%.1f lb".format(peso)
            tvMasaMagra.text    = "%.1f lb".format(masaMagra)
            tvMasaMuscular.text = "%.1f lb".format(masaMuscular)
            tvMasaGrasa.text    = "%.1f lb".format(masaGrasa)
            tvMasaOsea.text     = "%.1f lb".format(masaOsea)
            tvAgua.text         = "%.1f %%".format(agua)

            // Barras de progreso (% del peso total)
            if (peso > 0) {
                progressMasaMagra.progress    = ((masaMagra / peso) * 100).toInt()
                progressMasaMuscular.progress = ((masaMuscular / peso) * 100).toInt()
                progressMasaGrasa.progress    = ((masaGrasa / peso) * 100).toInt()
            }

            // ── MÉTRICAS EXTRA ────────────────────────────────────
            tvEdadMetabolica.text = "$edadMet años"
            tvGrasaVisceral.text  = "%.1f".format(grasaVis)
            tvPuntuacion.text     = "%.0f/100".format(puntuacion)
            tvTipoCuerpo.text     = tipoCuerpo
        }

        // ── CALORÍAS DEL DÍA ──────────────────────────────────────
        val fechaHoy = SimpleDateFormat("yyyy-MM-dd", Locale.getDefault()).format(Date())
        val calorias = obtenerCaloriasDia(usuarioId, fechaHoy)
        val calObjetivo = medicion[3].toFloatOrNull() ?: 2000f

        tvCalorias.text = "%.0f".format(calorias)
        tvCaloriasConsumidas.text = "%.0f".format(calorias)

        val progreso = if (calObjetivo > 0) ((calorias / calObjetivo) * 100).toInt().coerceIn(0, 100) else 0
        progressCalorias.progress = progreso

        val restantes = calObjetivo - calorias
        if (restantes >= 0) {
            tvCaloriasRestantes.text = "%.0f kcal restantes".format(restantes)
            tvCaloriasEstado.text = "✓ Vas bien, te quedan %.0f kcal".format(restantes)
        } else {
            tvCaloriasRestantes.text = "%.0f kcal excedidas".format(-restantes)
            tvCaloriasEstado.text = "⚠ Excediste tu objetivo por %.0f kcal".format(-restantes)
        }

        // ── BOTONES ───────────────────────────────────────────────
        btnRegistrarComida.setOnClickListener {
            startActivity(Intent(this, RegistroComidaActivity::class.java))
        }

        btnReportes.setOnClickListener {
            // TODO: ReportesActivity
        }
    }

    external fun inicializarDB(ruta: String): Boolean
    external fun obtenerPerfilUsuario(usuarioId: Int): Array<String>
    external fun obtenerUltimaMedicion(usuarioId: Int): Array<String>
    external fun obtenerCaloriasDia(usuarioId: Int, fecha: String): Float

    companion object {
        init { System.loadLibrary("nutriapp") }
    }
}
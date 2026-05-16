package com.nutriapp.android

import android.content.Intent
import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.cardview.widget.CardView

class ObjetivosActivity : AppCompatActivity() {

    private var objetivoSeleccionado: String = ""
    private var nivelActividad: String = "Sedentario"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_objetivos)

        val prefs     = getSharedPreferences("NutriApp", MODE_PRIVATE)
        val usuarioId = prefs.getInt("usuario_id", -1)

        if (usuarioId == -1) {
            startActivity(Intent(this, LoginActivity::class.java))
            finish()
            return
        }

        // Cards de objetivos
        val cardPerder    = findViewById<CardView>(R.id.cardPerder)
        val cardMantener  = findViewById<CardView>(R.id.cardMantener)
        val cardGanar     = findViewById<CardView>(R.id.cardGanar)
        val cardDefinir   = findViewById<CardView>(R.id.cardDefinir)
        val cardVolumen   = findViewById<CardView>(R.id.cardVolumen)

        // Cards de nivel de actividad
        val cardSedentario  = findViewById<CardView>(R.id.cardSedentario)
        val cardLigero      = findViewById<CardView>(R.id.cardLigero)
        val cardModerado    = findViewById<CardView>(R.id.cardModerado)
        val cardActivo      = findViewById<CardView>(R.id.cardActivo)
        val cardMuyActivo   = findViewById<CardView>(R.id.cardMuyActivo)

        val btnContinuar = findViewById<Button>(R.id.btnContinuar)
        val tvError      = findViewById<TextView>(R.id.tvError)

        // Listeners objetivos
        val cardsObjetivo = listOf(cardPerder, cardMantener, cardGanar, cardDefinir, cardVolumen)
        val objetivos     = listOf("Perder peso", "Mantener peso", "Ganar peso", "Definición", "Volumen")

        cardsObjetivo.forEachIndexed { i, card ->
            card.setOnClickListener {
                objetivoSeleccionado = objetivos[i]
                cardsObjetivo.forEach { it.cardElevation = 4f }
                card.cardElevation = 12f
                tvError.text = ""
            }
        }

        // Listeners nivel actividad
        val cardsActividad = listOf(cardSedentario, cardLigero, cardModerado, cardActivo, cardMuyActivo)
        val niveles        = listOf("Sedentario", "Ligero", "Moderado", "Activo", "Muy activo")

        cardsActividad.forEachIndexed { i, card ->
            card.setOnClickListener {
                nivelActividad = niveles[i]
                cardsActividad.forEach { it.cardElevation = 4f }
                card.cardElevation = 12f
            }
        }

        btnContinuar.setOnClickListener {
            if (objetivoSeleccionado.isEmpty()) {
                tvError.text = "Selecciona un objetivo"
                return@setOnClickListener
            }

            val exito = guardarObjetivo(usuarioId, objetivoSeleccionado, nivelActividad)

            if (exito) {
                registrarLog(usuarioId, "OBJETIVO_GUARDADO",
                    "Objetivo: $objetivoSeleccionado | Actividad: $nivelActividad",
                    "Objetivos", 1)
                startActivity(Intent(this, MainActivity::class.java))
                finish()
            } else {
                tvError.text = "Error al guardar el objetivo"
            }
        }
    }

    external fun guardarObjetivo(usuarioId: Int, objetivo: String, nivelActividad: String): Boolean
    external fun registrarLog(usuarioId: Int, accion: String,
                              detalle: String, pantalla: String, exitoso: Int): Boolean

    companion object {
        init { System.loadLibrary("nutriapp") }
    }
}
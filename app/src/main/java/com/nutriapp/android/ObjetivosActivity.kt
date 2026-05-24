package com.nutriapp.android

import android.content.Intent
import android.graphics.Color
import android.os.Bundle
import android.widget.Button
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity
import androidx.cardview.widget.CardView

class ObjetivosActivity : AppCompatActivity() {

    private var objetivoSeleccionado: String = ""
    private var nivelActividad: String = "Sedentario"

    // --- VARIABLES PARA CONTROLAR EL DISEÑO DE SELECCIÓN ---
    private var cardObjetivoSeleccionado: CardView? = null
    private var cardActividadSeleccionada: CardView? = null

    // Usamos el verde claro para la selección y blanco para el estado normal
    private val colorSeleccionado = Color.parseColor("#E8F5E9")
    private val colorOriginalFondo = Color.parseColor("#FFFFFF")

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

                // 1. Apaga la tarjeta anterior de objetivos
                cardObjetivoSeleccionado?.let { resaltarTarjeta(it, false) }
                // 2. Enciende la nueva tarjeta presionada
                resaltarTarjeta(card, true)
                // 3. Actualiza la referencia actual
                cardObjetivoSeleccionado = card

                tvError.text = ""
            }
        }

        // Listeners nivel actividad
        val cardsActividad = listOf(cardSedentario, cardLigero, cardModerado, cardActivo, cardMuyActivo)
        val niveles        = listOf("Sedentario", "Ligero", "Moderado", "Activo", "Muy activo")

        cardsActividad.forEachIndexed { i, card ->
            card.setOnClickListener {
                nivelActividad = niveles[i]

                // 1. Apaga la tarjeta anterior de actividad
                cardActividadSeleccionada?.let { resaltarTarjeta(it, false) }
                // 2. Enciende la nueva tarjeta presionada
                resaltarTarjeta(card, true)
                // 3. Actualiza la referencia actual
                cardActividadSeleccionada = card
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

    // --- FUNCIÓN CORREGIDA SIN PROPIEDADES PROBLEMÁTICAS ---
    private fun resaltarTarjeta(card: CardView, seleccionar: Boolean) {
        if (seleccionar) {
            card.setCardBackgroundColor(colorSeleccionado)
            card.cardElevation = 10f // Se levanta para denotar selección física
        } else {
            card.setCardBackgroundColor(colorOriginalFondo)
            card.cardElevation = 4f  // Regresa a su elevación sutil por defecto
        }
    }

    external fun guardarObjetivo(usuarioId: Int, objetivo: String, nivelActividad: String): Boolean
    external fun registrarLog(usuarioId: Int, accion: String,
                              detalle: String, pantalla: String, exitoso: Int): Boolean

    companion object {
        init { System.loadLibrary("nutriapp") }
    }
}
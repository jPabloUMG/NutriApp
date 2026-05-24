package com.nutriapp.android

import android.content.Intent
import android.os.Bundle
import android.widget.*
import androidx.appcompat.app.AppCompatActivity

class PerfilActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_perfil)

        val prefs     = getSharedPreferences("NutriApp", MODE_PRIVATE)
        val usuarioId = prefs.getInt("usuario_id", -1)

        if (usuarioId == -1) {
            startActivity(Intent(this, LoginActivity::class.java))
            finish()
            return
        }

        val etNombre   = findViewById<EditText>(R.id.etNombre)
        val etEdad     = findViewById<EditText>(R.id.etEdad)
        val spGenero   = findViewById<AutoCompleteTextView>(R.id.spGenero)
        val etAltura   = findViewById<EditText>(R.id.etAltura)
        val etPeso     = findViewById<EditText>(R.id.etPeso)
        val etGrasa    = findViewById<EditText>(R.id.etGrasa)
        val btnGuardar = findViewById<Button>(R.id.btnGuardar)
        val tvError    = findViewById<TextView>(R.id.tvError)

        val generos = arrayOf("Masculino", "Femenino")

        val adapter = ArrayAdapter(this, android.R.layout.simple_dropdown_item_1line, generos)

        spGenero.setAdapter(adapter)

        spGenero.inputType = android.text.InputType.TYPE_NULL

        btnGuardar.setOnClickListener {
            val nombre = etNombre.text.toString().trim()
            val edad   = etEdad.text.toString().trim()
            val genero = spGenero.text.toString()
            val altura = etAltura.text.toString().trim()
            val peso   = etPeso.text.toString().trim()
            val grasa  = etGrasa.text.toString().trim()

            if (nombre.isEmpty() || edad.isEmpty() || genero.isEmpty() || altura.isEmpty()
                || peso.isEmpty() || grasa.isEmpty()) {
                tvError.text = "Completa todos los campos, incluyendo el género"
                return@setOnClickListener
            }

            try {
                val edadInt = edad.toInt()
                val alturaFloat = altura.toFloat()
                val pesoFloat = peso.toFloat()
                val grasaFloat = grasa.toFloat()

                val exito = guardarPerfil(
                    usuarioId,
                    nombre,
                    edadInt,
                    genero,
                    alturaFloat,
                    pesoFloat,
                    grasaFloat
                )

                if (exito) {
                    registrarLog(usuarioId, "PERFIL_CREADO", "Perfil inicial guardado", "Perfil", 1)
                    startActivity(Intent(this, ObjetivosActivity::class.java))
                    finish()
                } else {
                    tvError.text = "Error de base de datos al guardar el perfil"
                }

            } catch (e: NumberFormatException) {
                tvError.text = "Por favor, introduce números válidos en Edad, Altura, Peso y Grasa"
            }
        }
    }

    external fun guardarPerfil(
        usuarioId: Int, nombre: String, edad: Int,
        genero: String, altura: Float, peso: Float, grasa: Float
    ): Boolean

    external fun registrarLog(usuarioId: Int, accion: String,
                              detalle: String, pantalla: String, exitoso: Int): Boolean

    companion object {
        init {
            System.loadLibrary("nutriapp")
        }
    }
}
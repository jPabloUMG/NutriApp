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
        val spGenero   = findViewById<Spinner>(R.id.spGenero)
        val etAltura   = findViewById<EditText>(R.id.etAltura)
        val etPeso     = findViewById<EditText>(R.id.etPeso)
        val etGrasa    = findViewById<EditText>(R.id.etGrasa)
        val btnGuardar = findViewById<Button>(R.id.btnGuardar)
        val tvError    = findViewById<TextView>(R.id.tvError)

        // Spinner de género
        val generos = arrayOf("Masculino", "Femenino")
        spGenero.adapter = ArrayAdapter(this, android.R.layout.simple_spinner_dropdown_item, generos)

        btnGuardar.setOnClickListener {
            val nombre = etNombre.text.toString().trim()
            val edad   = etEdad.text.toString().trim()
            val genero = spGenero.selectedItem.toString()
            val altura = etAltura.text.toString().trim()
            val peso   = etPeso.text.toString().trim()
            val grasa  = etGrasa.text.toString().trim()

            if (nombre.isEmpty() || edad.isEmpty() || altura.isEmpty()
                || peso.isEmpty() || grasa.isEmpty()) {
                tvError.text = "Completa todos los campos"
                return@setOnClickListener
            }

            val exito = guardarPerfil(
                usuarioId,
                nombre,
                edad.toInt(),
                genero,
                altura.toFloat(),
                peso.toFloat(),
                grasa.toFloat()
            )

            if (exito) {
                registrarLog(usuarioId, "PERFIL_CREADO", "Perfil inicial guardado", "Perfil", 1)
                startActivity(Intent(this, ObjetivosActivity::class.java))
                finish()
            } else {
                tvError.text = "Error al guardar el perfil"
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
        init { System.loadLibrary("nutriapp") }
    }
}
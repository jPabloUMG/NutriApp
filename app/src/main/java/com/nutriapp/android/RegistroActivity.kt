package com.nutriapp.android

import android.content.Intent
import android.os.Bundle
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import androidx.appcompat.app.AppCompatActivity

class RegistroActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_registro)

        // Inicializar base de datos
        val dbPath = filesDir.absolutePath + "/nutriapp.db"
        inicializarDB(dbPath)

        val etCorreo     = findViewById<EditText>(R.id.etCorreo)
        val etContrasena = findViewById<EditText>(R.id.etContrasena)
        val etConfirmar  = findViewById<EditText>(R.id.etConfirmar)
        val btnRegistrar = findViewById<Button>(R.id.btnRegistrar)
        val tvLogin      = findViewById<TextView>(R.id.tvLogin)
        val tvError      = findViewById<TextView>(R.id.tvError)

        btnRegistrar.setOnClickListener {
            val correo    = etCorreo.text.toString().trim()
            val pass      = etContrasena.text.toString().trim()
            val confirmar = etConfirmar.text.toString().trim()

            if (correo.isEmpty() || pass.isEmpty() || confirmar.isEmpty()) {
                tvError.text = "Completa todos los campos"
                return@setOnClickListener
            }
            if (pass != confirmar) {
                tvError.text = "Las contraseñas no coinciden"
                return@setOnClickListener
            }
            if (pass.length < 6) {
                tvError.text = "La contraseña debe tener al menos 6 caracteres"
                return@setOnClickListener
            }

            val usuarioId = registrarUsuario(correo, pass)

            when {
                usuarioId == -1 -> tvError.text = "El correo ya está registrado"
                usuarioId > 0   -> {
                    val prefs = getSharedPreferences("NutriApp", MODE_PRIVATE)
                    prefs.edit()
                        .putInt("usuario_id", usuarioId)
                        .putInt("rol_id", 2)
                        .putString("correo", correo)
                        .apply()

                    registrarLog(usuarioId, "REGISTRO", "Nueva cuenta creada", "Registro", 1)
                    startActivity(Intent(this, PerfilActivity::class.java))
                    finish()
                }
                else -> tvError.text = "Error al crear la cuenta"
            }
        }

        tvLogin.setOnClickListener { finish() }
    }

    external fun inicializarDB(ruta: String): Boolean
    external fun registrarUsuario(correo: String, contrasena: String): Int
    external fun registrarLog(usuarioId: Int, accion: String,
                              detalle: String, pantalla: String, exitoso: Int): Boolean

    companion object {
        init { System.loadLibrary("nutriapp") }
    }
}
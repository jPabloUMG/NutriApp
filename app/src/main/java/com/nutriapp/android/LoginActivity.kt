package com.nutriapp.android

import android.content.Intent
import android.os.Bundle
import android.widget.Button
import android.widget.EditText
import android.widget.TextView
import android.widget.Toast
import androidx.appcompat.app.AppCompatActivity

class LoginActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_login)

        val dbPath = filesDir.absolutePath + "/nutriapp.db"
        inicializarDB(dbPath)

        val etCorreo     = findViewById<EditText>(R.id.etCorreo)
        val etContrasena = findViewById<EditText>(R.id.etContrasena)
        val btnLogin     = findViewById<Button>(R.id.btnLogin)
        val tvRegistro   = findViewById<TextView>(R.id.tvRegistro)
        val tvError      = findViewById<TextView>(R.id.tvError)

        btnLogin.setOnClickListener {
            val correo    = etCorreo.text.toString().trim()
            val contrasena = etContrasena.text.toString().trim()

            if (correo.isEmpty() || contrasena.isEmpty()) {
                tvError.text = "Por favor completa todos los campos"
                return@setOnClickListener
            }

            val resultado = verificarLogin(correo, contrasena)

            if (resultado[0] == "-1") {
                tvError.text = "Correo o contraseña incorrectos"
                registrarLog(0, "LOGIN_FALLIDO", "Intento fallido: $correo", "Login", 0)
            } else {
                val usuarioId = resultado[0].toInt()
                val rolId     = resultado[1].toInt()

                registrarLog(usuarioId, "LOGIN_EXITOSO", "Login exitoso", "Login", 1)

                val prefs = getSharedPreferences("NutriApp", MODE_PRIVATE)
                prefs.edit()
                    .putInt("usuario_id", usuarioId)
                    .putInt("rol_id", rolId)
                    .putString("correo", correo)
                    .apply()

                if (rolId == 1) {
                    // SuperAdmin → AdminActivity
                    Toast.makeText(this, "Bienvenido Admin", Toast.LENGTH_SHORT).show()
                    startActivity(Intent(this, AdminActivity::class.java))
                } else {
                    // Usuario normal → verificar si tiene perfil
                    val tienePerfil = verificarPerfil(usuarioId)
                    if (tienePerfil) {
                        startActivity(Intent(this, MainActivity::class.java))
                    } else {
                        startActivity(Intent(this, PerfilActivity::class.java))
                    }
                }
                finish()
            }
        }

        tvRegistro.setOnClickListener {
            startActivity(Intent(this, RegistroActivity::class.java))
        }
    }

    external fun inicializarDB(ruta: String): Boolean
    external fun verificarLogin(correo: String, contrasena: String): Array<String>
    external fun verificarPerfil(usuarioId: Int): Boolean
    external fun registrarLog(usuarioId: Int, accion: String,
                              detalle: String, pantalla: String, exitoso: Int): Boolean

    companion object {
        init { System.loadLibrary("nutriapp") }
    }
}

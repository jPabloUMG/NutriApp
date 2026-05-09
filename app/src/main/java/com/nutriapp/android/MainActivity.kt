package com.nutriapp.android

import android.os.Bundle

import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Inicializar base de datos
        val dbPath = filesDir.absolutePath + "/nutriapp.db"
        inicializarDB(dbPath)
    }

    // ── Funciones C++ ──────────────────────────────────────────
    external fun inicializarDB(ruta: String): Boolean

    external fun insertarUsuario(
        nombre: String, edad: Int, genero: String,
        departamento: String, municipio: String,
        peso: Float, altura: Float,
        actividad: String, ocupacion: String, meta: String
    ): Boolean

    external fun calcularIMC(peso: Float, altura: Float): Float

    external fun calcularTMB(
        peso: Float, altura: Float,
        edad: Int, genero: String
    ): Float

    external fun calcularCaloriasRecomendadas(
        tmb: Float, actividad: String
    ): Float

    external fun generarRecomendaciones(
        peso: Float, altura: Float, edad: Int,
        genero: String, actividad: String, meta: String,
        calorias: Float, carbs: Float,
        proteinas: Float, grasas: Float, azucares: Float
    ): String

    companion object {
        init {
            System.loadLibrary("nutriapp")
        }
    }
}
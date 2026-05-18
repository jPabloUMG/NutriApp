package com.nutriapp.android

import android.content.Intent
import android.os.Bundle
import android.widget.Button
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

        val tvSaludo           = findViewById<TextView>(R.id.tvSaludo)
        val tvFecha            = findViewById<TextView>(R.id.tvFecha)
        val tvCalorias         = findViewById<TextView>(R.id.tvCalorias)
        val tvCaloriasObjetivo = findViewById<TextView>(R.id.tvCaloriasObjetivo)
        val tvIMC              = findViewById<TextView>(R.id.tvIMC)
        val tvPeso             = findViewById<TextView>(R.id.tvPeso)
        val tvGrasa            = findViewById<TextView>(R.id.tvGrasa)
        val btnRegistrarComida = findViewById<Button>(R.id.btnRegistrarComida)
        val btnReportes        = findViewById<Button>(R.id.btnReportes)

        val hoy = SimpleDateFormat("EEEE, dd 'de' MMMM", Locale("es")).format(Date())
        tvFecha.text = hoy.replaceFirstChar { it.uppercase() }

        val medicion = obtenerUltimaMedicion(usuarioId)
        if (medicion[0] != "-1") {
            tvIMC.text   = medicion[0]
            tvPeso.text  = "${medicion[1]} lb"
            tvGrasa.text = "${medicion[2]} %"
            tvCaloriasObjetivo.text = "Objetivo: ${medicion[3]} kcal"
        }

        val fechaHoy = SimpleDateFormat("yyyy-MM-dd", Locale.getDefault()).format(Date())
        val calorias = obtenerCaloriasDia(usuarioId, fechaHoy)
        tvCalorias.text = "${"%.0f".format(calorias)} kcal"

        btnRegistrarComida.setOnClickListener {
            startActivity(Intent(this, RegistroComidaActivity::class.java))
        }

        btnReportes.setOnClickListener {
            // TODO: ReportesActivity
        }
    }

    external fun inicializarDB(ruta: String): Boolean
    external fun obtenerUltimaMedicion(usuarioId: Int): Array<String>
    external fun obtenerCaloriasDia(usuarioId: Int, fecha: String): Float

    companion object {
        init { System.loadLibrary("nutriapp") }
    }
}
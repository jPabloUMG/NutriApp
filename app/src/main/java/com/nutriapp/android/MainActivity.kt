package com.nutriapp.android

import android.os.Bundle
import android.widget.*
import androidx.appcompat.app.AppCompatActivity

class MainActivity : AppCompatActivity() {

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        // Inicializar base de datos
        val dbPath = filesDir.absolutePath + "/nutriapp.db"
        inicializarDB(dbPath)

        // Referencias a los campos
        val etNombre = findViewById<EditText>(R.id.etNombre)
        val etEdad = findViewById<EditText>(R.id.etEdad)
        val etPeso = findViewById<EditText>(R.id.etPeso)
        val etAltura = findViewById<EditText>(R.id.etAltura)
        val spinnerGenero = findViewById<Spinner>(R.id.spinnerGenero)
        val spinnerActividad = findViewById<Spinner>(R.id.spinnerActividad)
        val spinnerMeta = findViewById<Spinner>(R.id.spinnerMeta)
        val btnRegistrar = findViewById<Button>(R.id.btnRegistrar)
        val tvResultado = findViewById<TextView>(R.id.tvResultado)

        // Opciones de los spinners
        val opcionesGenero = listOf("masculino", "femenino")
        val opcionesActividad = listOf("sedentario", "ligero", "moderado", "activo", "muy activo")
        val opcionesMeta = listOf("bajar", "mantener", "subir")

        spinnerGenero.adapter = ArrayAdapter(this,
            android.R.layout.simple_spinner_dropdown_item, opcionesGenero)
        spinnerActividad.adapter = ArrayAdapter(this,
            android.R.layout.simple_spinner_dropdown_item, opcionesActividad)
        spinnerMeta.adapter = ArrayAdapter(this,
            android.R.layout.simple_spinner_dropdown_item, opcionesMeta)

        // Botón registrar
        btnRegistrar.setOnClickListener {
            val nombre = etNombre.text.toString()
            val edad = etEdad.text.toString().toIntOrNull() ?: 0
            val peso = etPeso.text.toString().toFloatOrNull() ?: 0f
            val altura = etAltura.text.toString().toFloatOrNull() ?: 0f
            val genero = spinnerGenero.selectedItem.toString()
            val actividad = spinnerActividad.selectedItem.toString()
            val meta = spinnerMeta.selectedItem.toString()

            if (nombre.isEmpty() || peso == 0f || altura == 0f) {
                Toast.makeText(this, "Por favor completa todos los campos",
                    Toast.LENGTH_SHORT).show()
                return@setOnClickListener
            }

            // Guardar usuario
            insertarUsuario(nombre, edad, genero,
                "Guatemala", "Guatemala City",
                peso, altura, actividad, "Estudiante", meta)

            // Calcular IMC
            val imc = calcularIMC(peso, altura)
            val tmb = calcularTMB(peso, altura, edad, genero)
            val caloriasRec = calcularCaloriasRecomendadas(tmb, actividad)

            val categoriaIMC = when {
                imc < 18.5f -> "Bajo peso"
                imc < 25.0f -> "Normal"
                imc < 30.0f -> "Sobrepeso"
                else -> "Obesidad"
            }

            tvResultado.text = """
                ✓ Usuario registrado exitosamente
                
                IMC: ${"%.1f".format(imc)} ($categoriaIMC)
                TMB: ${"%.0f".format(tmb)} kcal
                Calorías recomendadas: ${"%.0f".format(caloriasRec)} kcal/día
            """.trimIndent()

            Toast.makeText(this, "¡Registro exitoso!", Toast.LENGTH_SHORT).show()
        }
    }

    external fun inicializarDB(ruta: String): Boolean
    external fun insertarUsuario(
        nombre: String, edad: Int, genero: String,
        departamento: String, municipio: String,
        peso: Float, altura: Float,
        actividad: String, ocupacion: String, meta: String
    ): Boolean
    external fun calcularIMC(peso: Float, altura: Float): Float
    external fun calcularTMB(peso: Float, altura: Float, edad: Int, genero: String): Float
    external fun calcularCaloriasRecomendadas(tmb: Float, actividad: String): Float
    external fun generarRecomendaciones(
        peso: Float, altura: Float, edad: Int,
        genero: String, actividad: String, meta: String,
        calorias: Float, carbs: Float,
        proteinas: Float, grasas: Float, azucares: Float
    ): String

    companion object {
        init { System.loadLibrary("nutriapp") }
    }
}
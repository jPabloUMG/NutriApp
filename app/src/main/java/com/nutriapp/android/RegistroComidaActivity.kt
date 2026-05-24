package com.nutriapp.android

import android.Manifest
import android.content.Intent
import android.content.pm.PackageManager
import android.graphics.Bitmap
import android.os.Bundle
import android.provider.MediaStore
import android.util.Base64
import android.view.View
import android.widget.*
import androidx.appcompat.app.AppCompatActivity
import androidx.core.app.ActivityCompat
import androidx.core.content.ContextCompat
import okhttp3.*
import okhttp3.MediaType.Companion.toMediaType
import okhttp3.RequestBody.Companion.toRequestBody
import org.json.JSONArray
import org.json.JSONObject
import java.io.ByteArrayOutputStream
import java.io.IOException

class RegistroComidaActivity : AppCompatActivity() {

    private var fotoBitmap: Bitmap? = null
    private val REQUEST_CAMERA = 100
    private val REQUEST_IMAGE_PICK = 101

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_registro_comida)

        val prefs     = getSharedPreferences("NutriApp", MODE_PRIVATE)
        val usuarioId = prefs.getInt("usuario_id", -1)

        if (usuarioId == -1) {
            finish()
            return
        }

        val dbPath = filesDir.absolutePath + "/nutriapp.db"
        inicializarDB(dbPath)

        val ivFoto         = findViewById<ImageView>(R.id.ivFoto)
        val btnCamara      = findViewById<Button>(R.id.btnCamara)
        val btnGaleria     = findViewById<Button>(R.id.btnGaleria)
        val btnAnalizar    = findViewById<Button>(R.id.btnAnalizar)
        val progressBar    = findViewById<ProgressBar>(R.id.progressBar)
        val layoutManual   = findViewById<LinearLayout>(R.id.layoutManual)
        val etNombre       = findViewById<EditText>(R.id.etNombre)
        val etCalorias     = findViewById<EditText>(R.id.etCalorias)
        val etProteinas    = findViewById<EditText>(R.id.etProteinas)
        val etCarbos       = findViewById<EditText>(R.id.etCarbos)
        val etGrasas       = findViewById<EditText>(R.id.etGrasas)
        val etCantidad     = findViewById<EditText>(R.id.etCantidad)
        val spTiempo       = findViewById<AutoCompleteTextView>(R.id.spTiempo)
        val btnGuardar     = findViewById<Button>(R.id.btnGuardar)
        val btnManual      = findViewById<Button>(R.id.btnManual)
        val tvError        = findViewById<TextView>(R.id.tvError)

        val tiempos = arrayOf("Desayuno", "Almuerzo", "Cena", "Merienda")
        val adapter = ArrayAdapter(this, android.R.layout.simple_dropdown_item_1line, tiempos)
        spTiempo.setAdapter(adapter)

        // ── Watcher para limpiar error mientras el usuario escribe ──────────────
        val camposTexto = listOf(etNombre, etCantidad, etCalorias, etProteinas, etCarbos, etGrasas)
        camposTexto.forEach { campo ->
            campo.addTextChangedListener(object : android.text.TextWatcher {
                override fun afterTextChanged(s: android.text.Editable?) {
                    if (tvError.text.isNotEmpty()) tvError.text = ""
                }
                override fun beforeTextChanged(s: CharSequence?, start: Int, count: Int, after: Int) {}
                override fun onTextChanged(s: CharSequence?, start: Int, before: Int, count: Int) {}
            })
        }

        // ── Cámara ──────────────────────────────────────────────────────────────
        btnCamara.setOnClickListener {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this,
                    arrayOf(Manifest.permission.CAMERA), REQUEST_CAMERA)
            } else {
                abrirCamara()
            }
        }

        // ── Galería ─────────────────────────────────────────────────────────────
        btnGaleria.setOnClickListener {
            val intent = Intent(Intent.ACTION_PICK, MediaStore.Images.Media.EXTERNAL_CONTENT_URI)
            startActivityForResult(intent, REQUEST_IMAGE_PICK)
        }

        // ── Analizar con IA ─────────────────────────────────────────────────────
        btnAnalizar.setOnClickListener {
            val bmp = fotoBitmap
            if (bmp == null) {
                tvError.text = "⚠ Primero toma o selecciona una foto"
                return@setOnClickListener
            }
            tvError.text = ""
            progressBar.visibility = View.VISIBLE
            btnAnalizar.isEnabled = false
            analizarConGemini(bmp, etNombre, etCalorias, etProteinas, etCarbos, etGrasas,
                progressBar, btnAnalizar, layoutManual, tvError)
        }

        // ── Ingresar manualmente ────────────────────────────────────────────────
        btnManual.setOnClickListener {
            layoutManual.visibility = View.VISIBLE
        }

        // ── Guardar comida — validación completa ────────────────────────────────
        btnGuardar.setOnClickListener {

            val tiempo    = spTiempo.text.toString().trim()
            val nombre    = etNombre.text.toString().trim()
            val cantidad  = etCantidad.text.toString().trim()
            val calorias  = etCalorias.text.toString().trim()
            val proteinas = etProteinas.text.toString().trim()
            val carbos    = etCarbos.text.toString().trim()
            val grasas    = etGrasas.text.toString().trim()

            // ── 1. Tiempo de comida ─────────────────────────────────────────────
            if (tiempo.isEmpty() || tiempo !in tiempos) {
                tvError.text = "⚠ Selecciona el tiempo de comida (Desayuno, Almuerzo, Cena o Merienda)"
                spTiempo.requestFocus()
                return@setOnClickListener
            }

            // ── 2. Nombre ───────────────────────────────────────────────────────
            if (nombre.isEmpty()) {
                tvError.text = "⚠ Ingresa el nombre del alimento"
                etNombre.requestFocus()
                return@setOnClickListener
            }
            if (nombre.length < 3) {
                tvError.text = "⚠ El nombre del alimento es demasiado corto"
                etNombre.requestFocus()
                return@setOnClickListener
            }

            // ── 3. Cantidad ─────────────────────────────────────────────────────
            if (cantidad.isEmpty()) {
                tvError.text = "⚠ Ingresa la cantidad (ej: 1 plato, 200g)"
                etCantidad.requestFocus()
                return@setOnClickListener
            }

            // ── 4. Calorías ─────────────────────────────────────────────────────
            if (calorias.isEmpty()) {
                tvError.text = "⚠ Ingresa las calorías del alimento"
                etCalorias.requestFocus()
                return@setOnClickListener
            }
            val caloriasFloat = calorias.toFloatOrNull()
            if (caloriasFloat == null || caloriasFloat < 0) {
                tvError.text = "⚠ Las calorías deben ser un número válido mayor o igual a 0"
                etCalorias.requestFocus()
                return@setOnClickListener
            }
            if (caloriasFloat > 5000) {
                tvError.text = "⚠ Las calorías ingresadas parecen demasiado altas. Verifica el dato"
                etCalorias.requestFocus()
                return@setOnClickListener
            }

            // ── 5. Proteínas ────────────────────────────────────────────────────
            if (proteinas.isEmpty()) {
                tvError.text = "⚠ Ingresa las proteínas (puedes poner 0 si no aplica)"
                etProteinas.requestFocus()
                return@setOnClickListener
            }
            val proteinasFloat = proteinas.toFloatOrNull()
            if (proteinasFloat == null || proteinasFloat < 0) {
                tvError.text = "⚠ Las proteínas deben ser un número válido mayor o igual a 0"
                etProteinas.requestFocus()
                return@setOnClickListener
            }

            // ── 6. Carbohidratos ────────────────────────────────────────────────
            if (carbos.isEmpty()) {
                tvError.text = "⚠ Ingresa los carbohidratos (puedes poner 0 si no aplica)"
                etCarbos.requestFocus()
                return@setOnClickListener
            }
            val carbosFloat = carbos.toFloatOrNull()
            if (carbosFloat == null || carbosFloat < 0) {
                tvError.text = "⚠ Los carbohidratos deben ser un número válido mayor o igual a 0"
                etCarbos.requestFocus()
                return@setOnClickListener
            }

            // ── 7. Grasas ───────────────────────────────────────────────────────
            if (grasas.isEmpty()) {
                tvError.text = "⚠ Ingresa las grasas (puedes poner 0 si no aplica)"
                etGrasas.requestFocus()
                return@setOnClickListener
            }
            val grasasFloat = grasas.toFloatOrNull()
            if (grasasFloat == null || grasasFloat < 0) {
                tvError.text = "⚠ Las grasas deben ser un número válido mayor o igual a 0"
                etGrasas.requestFocus()
                return@setOnClickListener
            }

            // ── 8. Validación cruzada de macros ─────────────────────────────────
            val totalMacrosKcal = (proteinasFloat * 4) + (carbosFloat * 4) + (grasasFloat * 9)
            if (caloriasFloat > 0 && totalMacrosKcal > 0) {
                val diferencia = Math.abs(caloriasFloat - totalMacrosKcal)
                val porcentajeDif = diferencia / caloriasFloat
                if (porcentajeDif > 0.5f) {
                    tvError.text = "⚠ Los macros ingresados no coinciden con las calorías. Verifica los datos"
                    return@setOnClickListener
                }
            }

            // ── Todos los campos válidos — guardar ──────────────────────────────
            tvError.text = ""
            val exito = guardarRegistroComida(
                usuarioId,
                nombre,
                cantidad,
                caloriasFloat,
                proteinasFloat,
                carbosFloat,
                grasasFloat,
                tiempo,
                if (fotoBitmap != null) "foto" else "manual"
            )

            if (exito) {
                Toast.makeText(this, "✅ Comida registrada correctamente", Toast.LENGTH_SHORT).show()
                finish()
            } else {
                tvError.text = "⚠ Error al guardar. Intenta de nuevo"
            }
        }
    }

    // ── Abrir cámara ────────────────────────────────────────────────────────────
    private fun abrirCamara() {
        val intent = Intent(MediaStore.ACTION_IMAGE_CAPTURE)
        startActivityForResult(intent, REQUEST_CAMERA)
    }

    // ── Resultado de cámara / galería ────────────────────────────────────────────
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        super.onActivityResult(requestCode, resultCode, data)
        val ivFoto = findViewById<ImageView>(R.id.ivFoto)

        if (resultCode == RESULT_OK) {
            when (requestCode) {
                REQUEST_CAMERA -> {
                    fotoBitmap = data?.extras?.get("data") as? Bitmap
                    ivFoto.setImageBitmap(fotoBitmap)
                }
                REQUEST_IMAGE_PICK -> {
                    val uri = data?.data
                    fotoBitmap = MediaStore.Images.Media.getBitmap(contentResolver, uri)
                    ivFoto.setImageBitmap(fotoBitmap)
                }
            }
        }
    }

    // ── Resultado de permisos ────────────────────────────────────────────────────
    override fun onRequestPermissionsResult(requestCode: Int,
                                            permissions: Array<out String>,
                                            grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == REQUEST_CAMERA && grantResults.isNotEmpty()
            && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            abrirCamara()
        }
    }

    // ── Analizar con Gemini ──────────────────────────────────────────────────────
    private fun analizarConGemini(
        bitmap: Bitmap,
        etNombre: EditText, etCalorias: EditText,
        etProteinas: EditText, etCarbos: EditText, etGrasas: EditText,
        progressBar: ProgressBar, btnAnalizar: Button,
        layoutManual: LinearLayout, tvError: TextView) {

        val baos = ByteArrayOutputStream()
        bitmap.compress(Bitmap.CompressFormat.JPEG, 80, baos)
        val imageBase64 = Base64.encodeToString(baos.toByteArray(), Base64.NO_WRAP)

        val prompt = """
            Analiza esta imagen de comida y responde ÚNICAMENTE con un JSON válido, sin texto extra, sin markdown.
            Formato exacto:
            {"nombre":"nombre del alimento","calorias":000,"proteinas":00,"carbohidratos":00,"grasas":00}
            Si no puedes identificar comida, responde: {"error":"no_food"}
        """.trimIndent()

        val requestJson = JSONObject().apply {
            put("contents", JSONArray().apply {
                put(JSONObject().apply {
                    put("parts", JSONArray().apply {
                        put(JSONObject().apply {
                            put("inline_data", JSONObject().apply {
                                put("mime_type", "image/jpeg")
                                put("data", imageBase64)
                            })
                        })
                        put(JSONObject().apply {
                            put("text", prompt)
                        })
                    })
                })
            })
        }

        val client = OkHttpClient()
        val body = requestJson.toString().toRequestBody("application/json".toMediaType())
        val request = Request.Builder()
            .url("${ApiConfig.GEMINI_URL}?key=${ApiConfig.GEMINI_API_KEY}")
            .post(body)
            .build()

        client.newCall(request).enqueue(object : Callback {
            override fun onFailure(call: Call, e: IOException) {
                runOnUiThread {
                    progressBar.visibility = View.GONE
                    btnAnalizar.isEnabled = true
                    tvError.text = "⚠ Error de conexión: ${e.message}"
                }
            }

            override fun onResponse(call: Call, response: Response) {
                val responseStr = response.body?.string() ?: ""
                runOnUiThread {
                    progressBar.visibility = View.GONE
                    btnAnalizar.isEnabled = true
                    try {
                        val jsonResponse = JSONObject(responseStr)
                        val text = jsonResponse
                            .getJSONArray("candidates")
                            .getJSONObject(0)
                            .getJSONObject("content")
                            .getJSONArray("parts")
                            .getJSONObject(0)
                            .getString("text")
                            .trim()

                        val cleanText = text
                            .replace("```json", "")
                            .replace("```", "")
                            .trim()

                        val foodData = JSONObject(cleanText)

                        if (foodData.has("error")) {
                            tvError.text = "⚠ No se pudo identificar la comida. Ingresa los datos manualmente"
                            layoutManual.visibility = View.VISIBLE
                        } else {
                            etNombre.setText(foodData.getString("nombre"))
                            etCalorias.setText(foodData.getInt("calorias").toString())
                            etProteinas.setText(foodData.getInt("proteinas").toString())
                            etCarbos.setText(foodData.getInt("carbohidratos").toString())
                            etGrasas.setText(foodData.getInt("grasas").toString())
                            layoutManual.visibility = View.VISIBLE
                        }
                    } catch (e: Exception) {
                        tvError.text = "⚠ Error al procesar respuesta de IA. Ingresa los datos manualmente"
                        layoutManual.visibility = View.VISIBLE
                    }
                }
            }
        })
    }

    external fun inicializarDB(ruta: String): Boolean

    external fun guardarRegistroComida(
        usuarioId: Int, nombre: String, cantidad: String,
        calorias: Float, proteinas: Float, carbohidratos: Float,
        grasas: Float, tiempoComida: String, fuente: String
    ): Boolean

    companion object {
        init { System.loadLibrary("nutriapp") }
    }
}
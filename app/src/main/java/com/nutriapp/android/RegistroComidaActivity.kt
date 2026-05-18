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
        val spTiempo       = findViewById<Spinner>(R.id.spTiempo)
        val btnGuardar     = findViewById<Button>(R.id.btnGuardar)
        val btnManual      = findViewById<Button>(R.id.btnManual)
        val tvError        = findViewById<TextView>(R.id.tvError)

        val tiempos = arrayOf("Desayuno", "Almuerzo", "Cena", "Merienda")
        spTiempo.adapter = ArrayAdapter(this, android.R.layout.simple_spinner_dropdown_item, tiempos)

        btnCamara.setOnClickListener {
            if (ContextCompat.checkSelfPermission(this, Manifest.permission.CAMERA)
                != PackageManager.PERMISSION_GRANTED) {
                ActivityCompat.requestPermissions(this,
                    arrayOf(Manifest.permission.CAMERA), REQUEST_CAMERA)
            } else {
                abrirCamara()
            }
        }

        btnGaleria.setOnClickListener {
            val intent = Intent(Intent.ACTION_PICK, MediaStore.Images.Media.EXTERNAL_CONTENT_URI)
            startActivityForResult(intent, REQUEST_IMAGE_PICK)
        }

        btnAnalizar.setOnClickListener {
            val bmp = fotoBitmap
            if (bmp == null) {
                tvError.text = "Primero toma o selecciona una foto"
                return@setOnClickListener
            }
            tvError.text = ""
            progressBar.visibility = View.VISIBLE
            btnAnalizar.isEnabled = false
            analizarConGemini(bmp, etNombre, etCalorias, etProteinas, etCarbos, etGrasas,
                progressBar, btnAnalizar, layoutManual, tvError)
        }

        btnManual.setOnClickListener {
            layoutManual.visibility = View.VISIBLE
        }

        btnGuardar.setOnClickListener {
            val nombre    = etNombre.text.toString().trim()
            val cantidad  = etCantidad.text.toString().trim()
            val calorias  = etCalorias.text.toString().trim()
            val proteinas = etProteinas.text.toString().trim()
            val carbos    = etCarbos.text.toString().trim()
            val grasas    = etGrasas.text.toString().trim()
            val tiempo    = spTiempo.selectedItem.toString()

            if (nombre.isEmpty() || calorias.isEmpty()) {
                tvError.text = "El nombre y calorías son obligatorios"
                return@setOnClickListener
            }

            val exito = guardarRegistroComida(
                usuarioId,
                nombre,
                cantidad.ifEmpty { "1 porción" },
                calorias.toFloatOrNull() ?: 0f,
                proteinas.toFloatOrNull() ?: 0f,
                carbos.toFloatOrNull() ?: 0f,
                grasas.toFloatOrNull() ?: 0f,
                tiempo,
                if (fotoBitmap != null) "foto" else "manual"
            )

            if (exito) {
                Toast.makeText(this, "✅ Comida registrada", Toast.LENGTH_SHORT).show()
                finish()
            } else {
                tvError.text = "Error al guardar"
            }
        }
    }

    private fun abrirCamara() {
        val intent = Intent(MediaStore.ACTION_IMAGE_CAPTURE)
        startActivityForResult(intent, REQUEST_CAMERA)
    }

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

    override fun onRequestPermissionsResult(requestCode: Int,
                                            permissions: Array<out String>,
                                            grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == REQUEST_CAMERA && grantResults.isNotEmpty()
            && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
            abrirCamara()
        }
    }

    private fun analizarConGemini(
        bitmap: Bitmap,
        etNombre: EditText, etCalorias: EditText,
        etProteinas: EditText, etCarbos: EditText, etGrasas: EditText,
        progressBar: ProgressBar, btnAnalizar: Button,
        layoutManual: LinearLayout, tvError: TextView) {

        val baos = ByteArrayOutputStream()
        bitmap.compress(Bitmap.CompressFormat.JPEG, 80, baos)
        val imageBase64 = Base64.encodeToString(baos.toByteArray(), Base64.NO_WRAP)

        android.util.Log.d("NutriApp", "Iniciando llamada a Gemini...")

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

        android.util.Log.d("NutriApp", "URL: ${ApiConfig.GEMINI_URL}")

        client.newCall(request).enqueue(object : Callback {

            override fun onFailure(call: Call, e: IOException) {
                android.util.Log.e("NutriApp", "onFailure: ${e.message}")
                runOnUiThread {
                    progressBar.visibility = View.GONE
                    btnAnalizar.isEnabled = true
                    tvError.text = "Error de conexión: ${e.message}"
                }
            }

            override fun onResponse(call: Call, response: Response) {
                val responseStr = response.body?.string() ?: ""
                android.util.Log.d("NutriApp", "HTTP ${response.code} - Respuesta: $responseStr")
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

                        android.util.Log.d("NutriApp", "Texto Gemini: $text")

                        // Limpiar markdown si Gemini lo devuelve
                        val cleanText = text
                            .replace("```json", "")
                            .replace("```", "")
                            .trim()

                        val foodData = JSONObject(cleanText)

                        if (foodData.has("error")) {
                            tvError.text = "No se pudo identificar la comida"
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
                        android.util.Log.e("NutriApp", "Error parseando: ${e.message}")
                        tvError.text = "Error al procesar respuesta de IA"
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
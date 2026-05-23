package com.nutriapp.android

import android.content.Intent
import android.graphics.Color
import android.os.Bundle
import android.view.Gravity
import android.widget.*
import androidx.appcompat.app.AppCompatActivity

class AdminActivity : AppCompatActivity() {

    private lateinit var tabUsuarios : LinearLayout
    private lateinit var tabLogs     : LinearLayout
    private lateinit var tabStats    : LinearLayout
    private lateinit var btnTabUsuarios: Button
    private lateinit var btnTabLogs    : Button
    private lateinit var btnTabStats   : Button

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_admin)

        val dbPath = filesDir.absolutePath + "/nutriapp.db"
        inicializarDB(dbPath)

        tabUsuarios   = findViewById(R.id.tabUsuarios)
        tabLogs       = findViewById(R.id.tabLogs)
        tabStats      = findViewById(R.id.tabStats)
        btnTabUsuarios = findViewById(R.id.btnTabUsuarios)
        btnTabLogs     = findViewById(R.id.btnTabLogs)
        btnTabStats    = findViewById(R.id.btnTabStats)

        btnTabUsuarios.setOnClickListener { mostrarTab(0) }
        btnTabLogs    .setOnClickListener { mostrarTab(1) }
        btnTabStats   .setOnClickListener { mostrarTab(2) }

        cargarDatos()
        mostrarTab(0)
    }

    private fun cargarDatos() {
        // ── USUARIOS ─────────────────────────────────────────────
        val usuariosRaw = obtenerTodosUsuariosStr()
        tabUsuarios.removeAllViews()

        if (usuariosRaw.isBlank()) {
            tabUsuarios.addView(textoVacio("Sin usuarios registrados"))
        } else {
            usuariosRaw.trim().split("\n").forEach { linea ->
                val partes = linea.split("|")
                if (partes.size >= 4) {
                    val id     = partes[0]
                    val correo = partes[1]
                    val rol    = if (partes[2] == "1") "SuperAdmin" else "Usuario"
                    val activo = partes[3] == "1"
                    tabUsuarios.addView(cardUsuario(id.toIntOrNull() ?: 0, correo, rol, activo))
                }
            }
        }

        // ── LOGS ─────────────────────────────────────────────────
        val logsRaw = obtenerTodosLogsStr()
        tabLogs.removeAllViews()

        if (logsRaw.isBlank()) {
            tabLogs.addView(textoVacio("Sin logs registrados"))
        } else {
            val lineas = logsRaw.trim().split("\n")
            // Mostrar máximo 100 logs para no saturar la UI
            lineas.take(100).forEach { linea ->
                val partes = linea.split("|")
                if (partes.size >= 6) {
                    val usuarioId = partes[0]
                    val accion    = partes[1]
                    val detalle   = partes[2]
                    val pantalla  = partes[3]
                    val fechaHora = partes[4]
                    val exitoso   = partes[5] == "1"
                    tabLogs.addView(cardLog(usuarioId, accion, detalle, pantalla, fechaHora, exitoso))
                }
            }
            if (lineas.size > 100) {
                tabLogs.addView(textoVacio("... y ${lineas.size - 100} logs más"))
            }
        }

        // ── STATS ─────────────────────────────────────────────────
        val stats = obtenerEstadisticasAdmin()
        tabStats.removeAllViews()
        // stats: "totalUsuarios|totalLogs|usuariosActivos"
        val partes = stats.split("|")
        val totalUsuarios  = partes.getOrNull(0) ?: "0"
        val totalLogs      = partes.getOrNull(1) ?: "0"
        val usuariosActivos= partes.getOrNull(2) ?: "0"

        tabStats.addView(cardStat("👥 Total usuarios",      totalUsuarios,  "#1565C0"))
        tabStats.addView(cardStat("✅ Usuarios activos",    usuariosActivos,"#2E7D32"))
        tabStats.addView(cardStat("📋 Total logs",          totalLogs,      "#E65100"))

        // Subtítulo header
        findViewById<TextView>(R.id.tvAdminSubtitulo).text =
            "$totalUsuarios usuarios · $totalLogs acciones registradas"
    }

    // ── CARD USUARIO ─────────────────────────────────────────────
    private fun cardUsuario(id: Int, correo: String, rol: String, activo: Boolean): LinearLayout {
        val card = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(16, 12, 16, 12)
            setBackgroundColor(Color.WHITE)
            val params = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT)
            params.setMargins(0, 0, 0, 8)
            layoutParams = params
        }

        val tvCorreo = TextView(this).apply {
            text = "✉ $correo"
            textSize = 14f
            setTextColor(Color.parseColor("#212121"))
            setTypeface(null, android.graphics.Typeface.BOLD)
        }

        val tvInfo = TextView(this).apply {
            text = "ID: $id  ·  Rol: $rol  ·  Estado: ${if (activo) "✅ Activo" else "❌ Inactivo"}"
            textSize = 12f
            setTextColor(Color.parseColor("#757575"))
        }

        val btnToggle = Button(this).apply {
            text = if (activo) "Desactivar" else "Activar"
            textSize = 12f
            setTextColor(Color.WHITE)
            setBackgroundColor(if (activo) Color.parseColor("#F44336") else Color.parseColor("#4CAF50"))
            val params = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.WRAP_CONTENT,
                LinearLayout.LayoutParams.WRAP_CONTENT)
            params.topMargin = 8
            layoutParams = params
            setOnClickListener {
                if (activo) {
                    desactivarUsuarioAdmin(id)
                } else {
                    activarUsuarioAdmin(id)
                }
                cargarDatos()
                mostrarTab(0)
                Toast.makeText(context,
                    if (activo) "Usuario desactivado" else "Usuario activado",
                    Toast.LENGTH_SHORT).show()
            }
        }

        card.addView(tvCorreo)
        card.addView(tvInfo)
        // No permitir desactivar superadmin (id==1) ni al propio admin logueado
        if (rol != "SuperAdmin") card.addView(btnToggle)

        return card
    }

    // ── CARD LOG ─────────────────────────────────────────────────
    private fun cardLog(
        usuarioId: String, accion: String, detalle: String,
        pantalla: String, fechaHora: String, exitoso: Boolean): LinearLayout {

        val card = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(12, 10, 12, 10)
            setBackgroundColor(if (exitoso) Color.parseColor("#F1F8E9") else Color.parseColor("#FBE9E7"))
            val params = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT)
            params.setMargins(0, 0, 0, 6)
            layoutParams = params
        }

        val tvAccion = TextView(this).apply {
            text = "${if (exitoso) "✅" else "❌"} $accion"
            textSize = 13f
            setTextColor(Color.parseColor("#212121"))
            setTypeface(null, android.graphics.Typeface.BOLD)
        }

        val tvDetalle = TextView(this).apply {
            text = "$detalle · $pantalla"
            textSize = 12f
            setTextColor(Color.parseColor("#616161"))
        }

        val tvMeta = TextView(this).apply {
            text = "Usuario #$usuarioId  ·  $fechaHora"
            textSize = 11f
            setTextColor(Color.parseColor("#9E9E9E"))
        }

        card.addView(tvAccion)
        card.addView(tvDetalle)
        card.addView(tvMeta)
        return card
    }

    // ── CARD STAT ─────────────────────────────────────────────────
    private fun cardStat(titulo: String, valor: String, color: String): LinearLayout {
        val card = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            setPadding(20, 20, 20, 20)
            setBackgroundColor(Color.parseColor(color))
            val params = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT)
            params.setMargins(0, 0, 0, 12)
            layoutParams = params
        }

        val tvTitulo = TextView(this).apply {
            text = titulo
            textSize = 15f
            setTextColor(Color.WHITE)
            layoutParams = LinearLayout.LayoutParams(0,
                LinearLayout.LayoutParams.WRAP_CONTENT, 1f)
        }

        val tvValor = TextView(this).apply {
            text = valor
            textSize = 28f
                    setTextColor(Color.WHITE)
            setTypeface(null, android.graphics.Typeface.BOLD)
            gravity = Gravity.END
        }

        card.addView(tvTitulo)
        card.addView(tvValor)
        return card
    }

    // ── TEXTO VACÍO ───────────────────────────────────────────────
    private fun textoVacio(msg: String): TextView {
        return TextView(this).apply {
            text = msg
            textSize = 14f
            setTextColor(Color.parseColor("#9E9E9E"))
            gravity = Gravity.CENTER
            setPadding(0, 40, 0, 40)
            layoutParams = LinearLayout.LayoutParams(
                LinearLayout.LayoutParams.MATCH_PARENT,
                LinearLayout.LayoutParams.WRAP_CONTENT)
        }
    }

    // ── CAMBIAR TAB ───────────────────────────────────────────────
    private fun mostrarTab(tab: Int) {
        tabUsuarios.visibility  = if (tab == 0) android.view.View.VISIBLE else android.view.View.GONE
        tabLogs.visibility      = if (tab == 1) android.view.View.VISIBLE else android.view.View.GONE
        tabStats.visibility     = if (tab == 2) android.view.View.VISIBLE else android.view.View.GONE

        val activeColor   = Color.parseColor("#1565C0")
        val inactiveColor = Color.parseColor("#1976D2")
        val activeText    = Color.WHITE
        val inactiveText  = Color.parseColor("#90CAF9")

        btnTabUsuarios.setBackgroundColor(if (tab == 0) activeColor else inactiveColor)
        btnTabLogs    .setBackgroundColor(if (tab == 1) activeColor else inactiveColor)
        btnTabStats   .setBackgroundColor(if (tab == 2) activeColor else inactiveColor)

        btnTabUsuarios.setTextColor(if (tab == 0) activeText else inactiveText)
        btnTabLogs    .setTextColor(if (tab == 1) activeText else inactiveText)
        btnTabStats   .setTextColor(if (tab == 2) activeText else inactiveText)
    }

    private val Int.sp get() = this.toFloat()

    external fun inicializarDB(ruta: String): Boolean
    external fun obtenerTodosUsuariosStr(): String
    external fun obtenerTodosLogsStr(): String
    external fun obtenerEstadisticasAdmin(): String
    external fun desactivarUsuarioAdmin(usuarioId: Int): Boolean
    external fun activarUsuarioAdmin(usuarioId: Int): Boolean


    companion object {
        init { System.loadLibrary("nutriapp") }
    }

}

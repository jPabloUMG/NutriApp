package com.nutriapp.android

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView

class ComidaAdapter(private val comidas: List<Comida>) :
    RecyclerView.Adapter<ComidaAdapter.ViewHolder>() {

    class ViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val tvIconoComida: TextView = view.findViewById(R.id.tvIconoComida)
        val tvFecha: TextView = view.findViewById(R.id.tvFecha)
        val tvTiempoComida: TextView = view.findViewById(R.id.tvTiempoComida)
        val tvNombreAlimento: TextView = view.findViewById(R.id.tvNombreAlimento)
        val tvCantidad: TextView = view.findViewById(R.id.tvCantidad)
        val tvCalorias: TextView = view.findViewById(R.id.tvCalorias)
        val tvMacros: TextView = view.findViewById(R.id.tvMacros)
        val tvFuente: TextView = view.findViewById(R.id.tvFuente)
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = LayoutInflater.from(parent.context)
            .inflate(R.layout.item_comida, parent, false)
        return ViewHolder(view)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        val comida = comidas[position]

        val icono = when (comida.tiempoComida.lowercase().trim()) {
            "desayuno" -> "🍳"
            "almuerzo" -> "🍗"
            "cena" -> "🌙"
            "merienda" -> "☕"
            else -> "🍽️"
        }

        // REGLA INFLEXIBLE: Elimina CUALQUIER número, guion o ":" al INICIO del nombre
        // Así si llega "36:54 - : Pizza", lo convierte en "Pizza"
        val destructoraDeNumeros = Regex("""^[\d\s:\-\–\—,]+""")
        val nombreLimpio = comida.nombre.replace(destructoraDeNumeros, "").trim()

        holder.tvIconoComida.text = icono
        holder.tvFecha.text = comida.fecha
        holder.tvTiempoComida.text = comida.tiempoComida
        holder.tvNombreAlimento.text = nombreLimpio.ifEmpty { "Sin nombre" }
        holder.tvCantidad.text = comida.cantidad
        holder.tvCalorias.text = "${comida.calorias.toInt()} kcal"

        if (comida.proteinas == 0f && comida.carbohidratos == 0f && comida.grasas == 0f) {
            holder.tvMacros.visibility = View.GONE
        } else {
            holder.tvMacros.visibility = View.VISIBLE
            holder.tvMacros.text = "Proteínas: ${comida.proteinas}g | Carbohidratos: ${comida.carbohidratos}g | Grasas: ${comida.grasas}g"
        }


        holder.tvFuente.text = "Fuente: ${comida.fuente}"
    }

    override fun getItemCount() = comidas.size
}

package com.nutriapp.android

data class Comida(
    val fecha: String,
    val tiempoComida: String,
    val nombre: String,
    val cantidad: String,
    val calorias: Float,
    val proteinas: Float,
    val carbohidratos: Float,
    val grasas: Float,
    val fuente: String
)

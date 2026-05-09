#ifndef NUTRIAPP_DATABASE_H
#define NUTRIAPP_DATABASE_H

#include <string>
#include <vector>
#include "sqlite3.h"

// Estructura Usuario
struct Usuario {
    int id;
    std::string nombre;
    int edad;
    std::string genero;
    std::string departamento;
    std::string municipio;
    float peso;
    float altura;
    std::string actividad;
    std::string ocupacion;
    std::string meta;
};

// Estructura Alimento
struct Alimento {
    int id;
    std::string nombre;
    float calorias;
    float carbohidratos;
    float proteinas;
    float grasas;
    float azucares;
    float grasas_sat;
    std::string fuente;
};

// Estructura Registro
struct Registro {
    int id;
    int usuario_id;
    int alimento_id;
    float cantidad;
    std::string unidad;
    std::string fecha;
    std::string hora;
    std::string tiempo_comida;
};

// Funciones de base de datos
bool inicializarDB(const std::string& ruta);
bool insertarUsuario(const Usuario& u);
bool insertarAlimento(const Alimento& a);
bool insertarRegistro(const Registro& r);
std::vector<Usuario> obtenerUsuarios();
std::vector<Alimento> obtenerAlimentos();
std::vector<Registro> obtenerRegistros(int usuario_id);
void cerrarDB();

#endif //NUTRIAPP_DATABASE_H
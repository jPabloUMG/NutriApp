#ifndef NUTRIAPP_DATABASE_H
#define NUTRIAPP_DATABASE_H

#include <string>
#include <vector>
#include "sqlite3.h"

// ── Estructuras ────────────────────────────────────────────────

struct Rol {
    int id;
    std::string nombre;
    std::string descripcion;
};

struct Usuario {
    int id;
    std::string correo;
    std::string contrasena;
    int rol_id;
    int activo;
    std::string fecha_registro;
    std::string ultimo_acceso;
};

struct Perfil {
    int id;
    int usuario_id;
    std::string nombre;
    int edad;
    std::string genero;
    float altura_cm;
    std::string nivel_actividad;
    std::string objetivo;
    int modo_pro;
    std::string fecha_creacion;
};

struct Medicion {
    int id;
    int usuario_id;
    std::string fecha;
    float peso_lb;
    float porcentaje_grasa;
    float imc;
    float tmb;
    float masa_grasa_lb;
    float masa_magra_lb;
    float masa_muscular_lb;
    float masa_osea_lb;
    float agua_corporal_pct;
    float grasa_visceral;
    int edad_metabolica;
    float puntuacion_salud;
    std::string tipo_cuerpo;
    float calorias_objetivo;
};

struct RegistroComida {
    int id;
    int usuario_id;
    std::string fecha;
    std::string hora;
    std::string tiempo_comida;
    std::string nombre_alimento;
    std::string cantidad;
    float calorias;
    float proteinas;
    float carbohidratos;
    float grasas;
    float azucares;
    std::string imagen_path;
    std::string fuente;
    int confirmado;
};

struct Log {
    int id;
    int usuario_id;
    std::string accion;
    std::string detalle;
    std::string pantalla;
    std::string fecha_hora;
    int exitoso;
};

struct Sesion {
    int id;
    int usuario_id;
    std::string token;
    std::string fecha_inicio;
    std::string fecha_expiracion;
    int activa;
    std::string dispositivo;
};

struct Reporte {
    int id;
    int usuario_id;
    std::string fecha_inicio;
    std::string fecha_fin;
    float calorias_promedio;
    float proteinas_promedio;
    float carbs_promedio;
    float grasas_promedio;
    float peso_inicial_lb;
    float peso_final_lb;
    std::string fecha_generado;
};

// ── Funciones de base de datos ─────────────────────────────────

// Inicialización
bool inicializarDB(const std::string& ruta);
void cerrarDB();

// Roles
bool inicializarRoles();

// Usuarios
bool insertarUsuario(const Usuario& u);
bool verificarLogin(const std::string& correo,
                    const std::string& contrasena,
                    Usuario& resultado);
bool actualizarUltimoAcceso(int usuario_id,
                            const std::string& fecha);
bool desactivarUsuario(int usuario_id);
std::vector<Usuario> obtenerTodosUsuarios();

// Perfil
bool insertarPerfil(const Perfil& p);
bool actualizarPerfil(const Perfil& p);
Perfil obtenerPerfil(int usuario_id);

// Mediciones
bool insertarMedicion(const Medicion& m);
std::vector<Medicion> obtenerMediciones(int usuario_id);
Medicion obtenerUltimaMedicion(int usuario_id);

// Registros de comida
bool insertarRegistroComida(const RegistroComida& r);
std::vector<RegistroComida> obtenerRegistrosComida(
        int usuario_id, const std::string& fecha);
float obtenerCaloriasDia(int usuario_id,
                         const std::string& fecha);

// Logs
bool insertarLog(const Log& l);
std::vector<Log> obtenerLogs(int usuario_id);
std::vector<Log> obtenerTodosLogs();

// Sesiones
bool insertarSesion(const Sesion& s);
bool cerrarSesion(const std::string& token);
bool validarSesion(const std::string& token);

// Reportes
bool insertarReporte(const Reporte& r);
std::vector<Reporte> obtenerReportes(int usuario_id);
std::vector<Reporte> obtenerTodosReportes();

#endif //NUTRIAPP_DATABASE_H
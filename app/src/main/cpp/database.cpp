//
// Created by JOSS on 2/05/2026.
//

#include "database.h"
#include <android/log.h>

#define LOG_TAG "NutriApp"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static sqlite3* db = nullptr;

bool inicializarDB(const std::string& ruta) {
    int result = sqlite3_open(ruta.c_str(), &db);
    if (result != SQLITE_OK) {
        LOGE("Error abriendo DB: %s", sqlite3_errmsg(db));
        return false;
    }

    const char* sql =
            "CREATE TABLE IF NOT EXISTS usuarios ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "nombre TEXT NOT NULL,"
            "edad INTEGER,"
            "genero TEXT,"
            "departamento TEXT,"
            "municipio TEXT,"
            "peso REAL,"
            "altura REAL,"
            "actividad TEXT,"
            "ocupacion TEXT,"
            "meta TEXT,"
            "fecha_registro TEXT);"

            "CREATE TABLE IF NOT EXISTS alimentos ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "nombre TEXT NOT NULL,"
            "calorias REAL,"
            "carbohidratos REAL,"
            "proteinas REAL,"
            "grasas REAL,"
            "azucares REAL,"
            "grasas_sat REAL,"
            "fuente TEXT);"

            "CREATE TABLE IF NOT EXISTS registros ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "usuario_id INTEGER,"
            "alimento_id INTEGER,"
            "cantidad REAL,"
            "unidad TEXT,"
            "fecha TEXT,"
            "hora TEXT,"
            "tiempo_comida TEXT,"
            "FOREIGN KEY(usuario_id) REFERENCES usuarios(id),"
            "FOREIGN KEY(alimento_id) REFERENCES alimentos(id));"

            "CREATE TABLE IF NOT EXISTS reportes ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "usuario_id INTEGER,"
            "fecha_inicio TEXT,"
            "fecha_fin TEXT,"
            "calorias_total REAL,"
            "carbs_total REAL,"
            "proteinas_total REAL,"
            "grasas_total REAL,"
            "imc REAL,"
            "fecha_generado TEXT,"
            "FOREIGN KEY(usuario_id) REFERENCES usuarios(id));";

    char* errMsg = nullptr;
    result = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (result != SQLITE_OK) {
        LOGE("Error creando tablas: %s", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool insertarUsuario(const Usuario& u) {
    const char* sql =
            "INSERT INTO usuarios (nombre, edad, genero, departamento, municipio, "
            "peso, altura, actividad, ocupacion, meta, fecha_registro) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, u.nombre.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, u.edad);
    sqlite3_bind_text(stmt, 3, u.genero.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, u.departamento.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, u.municipio.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 6, u.peso);
    sqlite3_bind_double(stmt, 7, u.altura);
    sqlite3_bind_text(stmt, 8, u.actividad.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 9, u.ocupacion.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 10, u.meta.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 11, "2026-01-01", -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

bool insertarAlimento(const Alimento& a) {
    const char* sql =
            "INSERT INTO alimentos (nombre, calorias, carbohidratos, proteinas, "
            "grasas, azucares, grasas_sat, fuente) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, a.nombre.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 2, a.calorias);
    sqlite3_bind_double(stmt, 3, a.carbohidratos);
    sqlite3_bind_double(stmt, 4, a.proteinas);
    sqlite3_bind_double(stmt, 5, a.grasas);
    sqlite3_bind_double(stmt, 6, a.azucares);
    sqlite3_bind_double(stmt, 7, a.grasas_sat);
    sqlite3_bind_text(stmt, 8, a.fuente.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

bool insertarRegistro(const Registro& r) {
    const char* sql =
            "INSERT INTO registros (usuario_id, alimento_id, cantidad, unidad, "
            "fecha, hora, tiempo_comida) "
            "VALUES (?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, r.usuario_id);
    sqlite3_bind_int(stmt, 2, r.alimento_id);
    sqlite3_bind_double(stmt, 3, r.cantidad);
    sqlite3_bind_text(stmt, 4, r.unidad.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, r.fecha.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, r.hora.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, r.tiempo_comida.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

void cerrarDB() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}
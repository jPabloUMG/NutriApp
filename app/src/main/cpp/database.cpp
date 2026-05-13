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
            "CREATE TABLE IF NOT EXISTS roles ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "nombre TEXT NOT NULL,"
            "descripcion TEXT);"

            "CREATE TABLE IF NOT EXISTS usuarios ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "correo TEXT UNIQUE NOT NULL,"
            "contrasena TEXT NOT NULL,"
            "rol_id INTEGER DEFAULT 2,"
            "activo INTEGER DEFAULT 1,"
            "fecha_registro TEXT,"
            "ultimo_acceso TEXT,"
            "FOREIGN KEY(rol_id) REFERENCES roles(id));"

            "CREATE TABLE IF NOT EXISTS perfil ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "usuario_id INTEGER UNIQUE,"
            "nombre TEXT,"
            "edad INTEGER,"
            "genero TEXT,"
            "altura_cm REAL,"
            "nivel_actividad TEXT,"
            "objetivo TEXT,"
            "modo_pro INTEGER DEFAULT 0,"
            "fecha_creacion TEXT,"
            "FOREIGN KEY(usuario_id) REFERENCES usuarios(id));"

            "CREATE TABLE IF NOT EXISTS mediciones ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "usuario_id INTEGER,"
            "fecha TEXT,"
            "peso_lb REAL,"
            "porcentaje_grasa REAL,"
            "imc REAL,"
            "tmb REAL,"
            "masa_grasa_lb REAL,"
            "masa_magra_lb REAL,"
            "masa_muscular_lb REAL,"
            "masa_osea_lb REAL,"
            "agua_corporal_pct REAL,"
            "grasa_visceral REAL,"
            "edad_metabolica INTEGER,"
            "puntuacion_salud REAL,"
            "tipo_cuerpo TEXT,"
            "calorias_objetivo REAL,"
            "FOREIGN KEY(usuario_id) REFERENCES usuarios(id));"

            "CREATE TABLE IF NOT EXISTS registros_comida ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "usuario_id INTEGER,"
            "fecha TEXT,"
            "hora TEXT,"
            "tiempo_comida TEXT,"
            "nombre_alimento TEXT,"
            "cantidad TEXT,"
            "calorias REAL,"
            "proteinas REAL,"
            "carbohidratos REAL,"
            "grasas REAL,"
            "azucares REAL,"
            "imagen_path TEXT,"
            "fuente TEXT,"
            "confirmado INTEGER DEFAULT 0,"
            "FOREIGN KEY(usuario_id) REFERENCES usuarios(id));"

            "CREATE TABLE IF NOT EXISTS logs ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "usuario_id INTEGER,"
            "accion TEXT,"
            "detalle TEXT,"
            "pantalla TEXT,"
            "fecha_hora TEXT,"
            "exitoso INTEGER DEFAULT 1,"
            "FOREIGN KEY(usuario_id) REFERENCES usuarios(id));"

            "CREATE TABLE IF NOT EXISTS sesiones ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "usuario_id INTEGER,"
            "token TEXT,"
            "fecha_inicio TEXT,"
            "fecha_expiracion TEXT,"
            "activa INTEGER DEFAULT 1,"
            "dispositivo TEXT,"
            "FOREIGN KEY(usuario_id) REFERENCES usuarios(id));"

            "CREATE TABLE IF NOT EXISTS reportes ("
            "id INTEGER PRIMARY KEY AUTOINCREMENT,"
            "usuario_id INTEGER,"
            "fecha_inicio TEXT,"
            "fecha_fin TEXT,"
            "calorias_promedio REAL,"
            "proteinas_promedio REAL,"
            "carbs_promedio REAL,"
            "grasas_promedio REAL,"
            "peso_inicial_lb REAL,"
            "peso_final_lb REAL,"
            "fecha_generado TEXT,"
            "FOREIGN KEY(usuario_id) REFERENCES usuarios(id));";

    char* errMsg = nullptr;
    result = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (result != SQLITE_OK) {
        LOGE("Error creando tablas: %s", errMsg);
        sqlite3_free(errMsg);
        return false;
    }

    return inicializarRoles();
}

bool inicializarRoles() {
    const char* sql =
            "INSERT OR IGNORE INTO roles (id, nombre, descripcion) VALUES "
            "(1, 'superadmin', 'Administrador con acceso total'),"
            "(2, 'usuario', 'Usuario regular de la app');";

    char* errMsg = nullptr;
    int result = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
    if (result != SQLITE_OK) {
        LOGE("Error insertando roles: %s", errMsg);
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

bool insertarUsuario(const Usuario& u) {
    const char* sql =
            "INSERT INTO usuarios (correo, contrasena, rol_id, activo, fecha_registro) "
            "VALUES (?, ?, ?, 1, ?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, u.correo.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, u.contrasena.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, u.rol_id);
    sqlite3_bind_text(stmt, 4, u.fecha_registro.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

bool verificarLogin(const std::string& correo,
                    const std::string& contrasena,
                    Usuario& resultado) {
    const char* sql =
            "SELECT id, correo, contrasena, rol_id, activo FROM usuarios "
            "WHERE correo = ? AND contrasena = ? AND activo = 1;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, correo.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, contrasena.c_str(), -1, SQLITE_STATIC);

    bool found = false;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        resultado.id = sqlite3_column_int(stmt, 0);
        resultado.correo = (const char*)sqlite3_column_text(stmt, 1);
        resultado.contrasena = (const char*)sqlite3_column_text(stmt, 2);
        resultado.rol_id = sqlite3_column_int(stmt, 3);
        resultado.activo = sqlite3_column_int(stmt, 4);
        found = true;
    }
    sqlite3_finalize(stmt);
    return found;
}

bool actualizarUltimoAcceso(int usuario_id, const std::string& fecha) {
    const char* sql = "UPDATE usuarios SET ultimo_acceso = ? WHERE id = ?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, fecha.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, usuario_id);
    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

bool desactivarUsuario(int usuario_id) {
    const char* sql = "UPDATE usuarios SET activo = 0 WHERE id = ?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, usuario_id);
    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

std::vector<Usuario> obtenerTodosUsuarios() {
    std::vector<Usuario> lista;
    const char* sql = "SELECT id, correo, rol_id, activo, fecha_registro, ultimo_acceso FROM usuarios;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Usuario u;
        u.id = sqlite3_column_int(stmt, 0);
        u.correo = (const char*)sqlite3_column_text(stmt, 1);
        u.rol_id = sqlite3_column_int(stmt, 2);
        u.activo = sqlite3_column_int(stmt, 3);
        u.fecha_registro = (const char*)sqlite3_column_text(stmt, 4);
        u.ultimo_acceso = sqlite3_column_text(stmt, 5) ?
                          (const char*)sqlite3_column_text(stmt, 5) : "";
        lista.push_back(u);
    }
    sqlite3_finalize(stmt);
    return lista;
}

bool insertarPerfil(const Perfil& p) {
    const char* sql =
            "INSERT OR REPLACE INTO perfil "
            "(usuario_id, nombre, edad, genero, altura_cm, nivel_actividad, objetivo, modo_pro, fecha_creacion) "
            "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, p.usuario_id);
    sqlite3_bind_text(stmt, 2, p.nombre.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 3, p.edad);
    sqlite3_bind_text(stmt, 4, p.genero.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 5, p.altura_cm);
    sqlite3_bind_text(stmt, 6, p.nivel_actividad.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, p.objetivo.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 8, p.modo_pro);
    sqlite3_bind_text(stmt, 9, p.fecha_creacion.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

bool insertarMedicion(const Medicion& m) {
    const char* sql =
            "INSERT INTO mediciones "
            "(usuario_id, fecha, peso_lb, porcentaje_grasa, imc, tmb, "
            "masa_grasa_lb, masa_magra_lb, masa_muscular_lb, masa_osea_lb, "
            "agua_corporal_pct, grasa_visceral, edad_metabolica, "
            "puntuacion_salud, tipo_cuerpo, calorias_objetivo) "
            "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, m.usuario_id);
    sqlite3_bind_text(stmt, 2, m.fecha.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 3, m.peso_lb);
    sqlite3_bind_double(stmt, 4, m.porcentaje_grasa);
    sqlite3_bind_double(stmt, 5, m.imc);
    sqlite3_bind_double(stmt, 6, m.tmb);
    sqlite3_bind_double(stmt, 7, m.masa_grasa_lb);
    sqlite3_bind_double(stmt, 8, m.masa_magra_lb);
    sqlite3_bind_double(stmt, 9, m.masa_muscular_lb);
    sqlite3_bind_double(stmt, 10, m.masa_osea_lb);
    sqlite3_bind_double(stmt, 11, m.agua_corporal_pct);
    sqlite3_bind_double(stmt, 12, m.grasa_visceral);
    sqlite3_bind_int(stmt, 13, m.edad_metabolica);
    sqlite3_bind_double(stmt, 14, m.puntuacion_salud);
    sqlite3_bind_text(stmt, 15, m.tipo_cuerpo.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 16, m.calorias_objetivo);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

bool insertarRegistroComida(const RegistroComida& r) {
    const char* sql =
            "INSERT INTO registros_comida "
            "(usuario_id, fecha, hora, tiempo_comida, nombre_alimento, cantidad, "
            "calorias, proteinas, carbohidratos, grasas, azucares, "
            "imagen_path, fuente, confirmado) "
            "VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, r.usuario_id);
    sqlite3_bind_text(stmt, 2, r.fecha.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, r.hora.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, r.tiempo_comida.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, r.nombre_alimento.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, r.cantidad.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_double(stmt, 7, r.calorias);
    sqlite3_bind_double(stmt, 8, r.proteinas);
    sqlite3_bind_double(stmt, 9, r.carbohidratos);
    sqlite3_bind_double(stmt, 10, r.grasas);
    sqlite3_bind_double(stmt, 11, r.azucares);
    sqlite3_bind_text(stmt, 12, r.imagen_path.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 13, r.fuente.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 14, r.confirmado);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

bool insertarLog(const Log& l) {
    const char* sql =
            "INSERT INTO logs (usuario_id, accion, detalle, pantalla, fecha_hora, exitoso) "
            "VALUES (?,?,?,?,?,?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, l.usuario_id);
    sqlite3_bind_text(stmt, 2, l.accion.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, l.detalle.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, l.pantalla.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, l.fecha_hora.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 6, l.exitoso);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

float obtenerCaloriasDia(int usuario_id, const std::string& fecha) {
    const char* sql =
            "SELECT SUM(calorias) FROM registros_comida "
            "WHERE usuario_id = ? AND fecha = ?;";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, usuario_id);
    sqlite3_bind_text(stmt, 2, fecha.c_str(), -1, SQLITE_STATIC);

    float total = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW)
        total = (float)sqlite3_column_double(stmt, 0);
    sqlite3_finalize(stmt);
    return total;
}

bool insertarSesion(const Sesion& s) {
    const char* sql =
            "INSERT INTO sesiones (usuario_id, token, fecha_inicio, fecha_expiracion, activa, dispositivo) "
            "VALUES (?,?,?,?,1,?);";

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_int(stmt, 1, s.usuario_id);
    sqlite3_bind_text(stmt, 2, s.token.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, s.fecha_inicio.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, s.fecha_expiracion.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, s.dispositivo.c_str(), -1, SQLITE_STATIC);

    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

bool cerrarSesion(const std::string& token) {
    const char* sql = "UPDATE sesiones SET activa = 0 WHERE token = ?;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_STATIC);
    int result = sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    return result == SQLITE_DONE;
}

bool validarSesion(const std::string& token) {
    const char* sql = "SELECT id FROM sesiones WHERE token = ? AND activa = 1;";
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    sqlite3_bind_text(stmt, 1, token.c_str(), -1, SQLITE_STATIC);
    bool valid = sqlite3_step(stmt) == SQLITE_ROW;
    sqlite3_finalize(stmt);
    return valid;
}

void cerrarDB() {
    if (db) {
        sqlite3_close(db);
        db = nullptr;
    }
}